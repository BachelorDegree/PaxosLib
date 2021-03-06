#include <vector>
#include <memory>
#include <iostream>
#include <spdlog/spdlog.h>

#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/role/learner.hpp"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "sys/eventfd.h"
#include "paxoslib/nodeimpl.hpp"
#include "paxoslib/util/timetrace.hpp"
#include "paxoslib/eventloop/eventtype.hpp"
namespace paxoslib
{

InstanceImpl::InstanceImpl(uint32_t iGroupIndex, NodeImpl *pNode, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork, std::unique_ptr<paxoslib::persistence::Storage> pStorage, std::shared_ptr<StateMachineMgr> pStateMachineMgr) : m_pNetwork(pNetwork), m_pStorage(std::move(pStorage)), m_oProposer(this, m_oContext), m_oAccepter(this), m_oLearner(this, m_oContext), m_pStateMachineMgr(pStateMachineMgr)
{
  m_pNode = pNode;
  m_iGroupIndex = iGroupIndex;
}
int InstanceImpl::Init()
{
  uint64_t ddwInstanceId = 0;
  int iRet = m_pStorage->GetMaxInstanceId(ddwInstanceId);
  assert(iRet == 0);
  uint64_t ddwCPInstanceId = 0;
  iRet = m_pStateMachineMgr->GetCheckpointMinInstanceID(m_iGroupIndex, ddwCPInstanceId);
  assert(iRet == 0);
  //execute to the last log - 1
  for (auto id = ddwCPInstanceId + 1; id < ddwInstanceId; id++)
  {
    paxoslib::StateProto oState;
    iRet = m_pStorage->LoadState(ddwInstanceId, oState);
    assert(iRet == 0);
    assert(oState.accepted());
    //TODO
    SPDLOG_DEBUG("Execute log for {}", id);
    iRet = m_pStateMachineMgr->ExecuteForReplay(m_iGroupIndex, 1, id, oState.accepted_proposal().value());
    assert(iRet == 0);
  }
  paxoslib::StateProto oLastState;
  iRet = m_pStorage->LoadState(ddwInstanceId, oLastState);
  assert(iRet == 404 || iRet == 0);
  m_oAccepter.SetState(oLastState);
  m_oProposer.SetInstanceID(ddwInstanceId);
  m_oAccepter.SetInstanceID(ddwInstanceId);
  m_oLearner.SetInstanceID(ddwInstanceId);
  if (iRet == 0 && oLastState.accepted())
  {
    SPDLOG_DEBUG("last log accepted");
    if (ddwCPInstanceId < ddwInstanceId)
    {
      SPDLOG_DEBUG("replay last log");
      SPDLOG_DEBUG("Execute log for {}", ddwInstanceId);
      iRet = m_pStateMachineMgr->ExecuteForReplay(m_iGroupIndex, 1, ddwInstanceId, oLastState.accepted_proposal().value());
      assert(iRet == 0);
    }
    else
    {
      SPDLOG_DEBUG("last log already executed. skip");
    }
    this->NewInstance();
  }
  SPDLOG_INFO("Instance Begin from {}", this->m_oAccepter.GetInstanceID());
  m_oLearner.AskForInstanceID();
  m_oEventLoop.Start();
  return 0;
}
struct Package
{
  char *pBuffer;
  uint32_t size;
};
void InstanceImpl::EnqueuePackageEventloop(char *pBuffer, uint32_t size)
{
  auto pPackage = new Package;
  pPackage->pBuffer = pBuffer;
  pPackage->size = size;
  this->m_oEventLoop.AddEventTail(this, eventloop::EventType::InstanceMessage, pPackage);
}
void InstanceImpl::OnEvent(int iEventType, void *data)
{
  switch (iEventType)
  {
  case eventloop::EventType::InstanceMessage:
  {
    Package *pPackage = reinterpret_cast<Package *>(data);
    Message oMessage;
    oMessage.ParseFromArray(pPackage->pBuffer + sizeof(uint32_t), pPackage->size - sizeof(uint32_t));
    delete pPackage->pBuffer;
    this->OnMessage(oMessage);
    delete pPackage;

    break;
  }
  default:
    SPDLOG_ERROR("Unknown event");
  }
}
uint64_t InstanceImpl::AddTimeout(uint64_t msTime, int iEventType, void *data)
{
  return this->m_oEventLoop.AddTimeout(this, msTime, iEventType, data);
}
uint32_t InstanceImpl::GetGroupIndex() const
{
  return m_iGroupIndex;
}
void InstanceImpl::RemoveTimeout(uint64_t id)
{
  this->m_oEventLoop.RemoveTimeout(id);
}
void InstanceImpl::OnTimeout(int iEventType, void *data)
{
  switch (iEventType)
  {
  case eventloop::EventType::LearnerAskForInstanceIdLoop:
  {
    m_oLearner.AskForInstanceID();
    break;
  }
  case eventloop::EventType::ProposeTimeout:
  {
    SPDLOG_ERROR("Propose Timeout");
    m_oContext.CommitResult(false, this->m_oProposer.GetInstanceID(), "");
    break;
  }
  default:
    SPDLOG_ERROR("Unknown timeout");
  }
}
int InstanceImpl::OnMessage(const Message &oMessage)
{
  Trace::Mark(oMessage.id(), "InstanceImpl::OnMessage");
  SPDLOG_DEBUG("OnMessage {}", oMessage.ShortDebugString());
  int iRet = 0;
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_PROMISED:
  case Message_Type::Message_Type_REJECT_PREOMISE:
  case Message_Type::Message_Type_ACCEPTED:
  case Message_Type::Message_Type_REJECT_ACCEPT:
    iRet = OnProposerMessage(oMessage);
    break;
  case Message_Type::Message_Type_ACCEPT:
  case Message_Type::Message_Type_PREAPRE:
    iRet = OnAccepterMessage(oMessage);
    break;
  case Message_Type::Message_Type_LEARN:
  case Message_Type::Message_Type_ASK_FOR_INSTANCE_ID:
  case Message_Type::Message_Type_ASK_FOR_INSTANCE_ID_REPLY:
  case Message_Type::Message_Type_ASK_FOR_LEARN_REPLY:
  case Message_Type::Message_Type_ASK_FOR_LEARN:
    iRet = OnLearnerMessage(oMessage);
    break;
  }
  return iRet;
}
int InstanceImpl::OnProposerMessage(const Message &oMessage)
{
  if (oMessage.instance_id() != m_oProposer.GetInstanceID())
  {
    //SPDLOG_ERROR("Not same Instance. my:{} his:{}", m_oProposer.GetInstanceID(), oMessage.instance_id());
    return -1;
  }
  return m_oProposer.OnMessage(oMessage);
}
int InstanceImpl::OnAccepterMessage(const Message &oMessage)
{
  if (oMessage.instance_id() != m_oProposer.GetInstanceID())
  {
    SPDLOG_ERROR("Not same Instance. my:{} his:{} from:{} to:{} {}", m_oProposer.GetInstanceID(), oMessage.instance_id(), oMessage.from_node_id(), oMessage.to_node_id(), oMessage.ShortDebugString());
    return -1;
  }
  return m_oAccepter.OnMessage(oMessage);
}
int InstanceImpl::OnLearnerMessage(const Message &oMessage)
{
  if (oMessage.type() == Message_Type_ASK_FOR_INSTANCE_ID ||
      oMessage.type() == Message_Type_ASK_FOR_INSTANCE_ID_REPLY ||
      oMessage.type() == Message_Type_ASK_FOR_LEARN_REPLY ||
      oMessage.type() == Message_Type_ASK_FOR_LEARN)
  {
    int iRet = m_oLearner.OnMessage(oMessage);
    if (iRet != 0)
    {
      SPDLOG_ERROR("m_oLearner.OnMessage failed. ret: {}", iRet);
      return iRet;
    }
    return 0;
  }
  if (oMessage.instance_id() != m_oProposer.GetInstanceID())
  {
    SPDLOG_ERROR("Not same Instance. my:{} his:{}", m_oProposer.GetInstanceID(), oMessage.instance_id());
    return -1;
  }
  int iRet = m_oLearner.OnMessage(oMessage);
  if (iRet != 0)
  {
    SPDLOG_ERROR("m_oLearner.OnMessage failed. ret: {}", iRet);
    return iRet;
  }
  if (m_oLearner.IsLearned())
  {
    iRet = this->OnLearnNewValue(m_oAccepter.GetInstanceID(), m_oAccepter.GetState().GetAcceptedProposal().value());
    if (iRet != 0)
    {
      SPDLOG_ERROR("OnLearnNewValue failed. ret: {}", iRet);
      return iRet;
    }
    if (oMessage.from_node_id() == GetNodeId())
    {
      this->RemoveTimeout(this->m_propose_timeout_id);
      auto id = m_oAccepter.GetInstanceID();
      auto value = m_oAccepter.GetState().GetAcceptedProposal().value();
      NewInstance();
      m_oContext.CommitResult(true, id, value);
    }
    else
    {
      NewInstance();
    }
  }
  return 0;
}
int InstanceImpl::OnLearnNewValue(uint64_t id, const std::string &value)
{
  //TODO
  return this->ExecuteStateMachine(id, value);
}
int InstanceImpl::ExecuteStateMachine(uint64_t id, const std::string &value)
{
  //TODO
  return this->m_pStateMachineMgr->Execute(m_iGroupIndex, 1, id, value);
}
uint16_t InstanceImpl::GetNodeId() const
{
  return this->m_pNode->GetNodeId();
}
int InstanceImpl::Propose(const std::string &value, uint64_t &ddwInstanceId)
{
  m_oContext.Reset(this->m_oProposer.GetInstanceID(), value);
  Proposal oProposal;
  oProposal.set_value(value);
  m_propose_timeout_id = this->AddTimeout(2000, eventloop::EventType::ProposeTimeout, 0);
  m_oProposer.Propose(oProposal);
  return m_oContext.WaitAndGetResult(ddwInstanceId);
}
std::vector<std::shared_ptr<network::Peer>> InstanceImpl::GetPeers() const
{
  return this->m_pNode->GetPeers();
}

void InstanceImpl::NewInstance()
{
  m_oProposer.NewInstance();
  m_oAccepter.NewInstance();
  m_oLearner.NewInstance();
}
}; // namespace paxoslib