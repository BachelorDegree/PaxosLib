#include <spdlog/spdlog.h>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/instanceimpl.hpp"
using namespace paxoslib;
namespace paxoslib::role
{
Proposer::Proposer(InstanceImpl *pInstance, Context &oContext) : Role(pInstance), m_oContext(oContext)
{
  m_oStage = Stage::Idle;
}
uint64_t Proposer::GetProposalId()
{
  return m_ddwProposalId;
}
uint64_t Proposer::RespawnProposalId()
{
  static std::atomic<uint16_t> rand{0};
  uint64_t proposal_id = 0;
  proposal_id += (uint64_t)time(nullptr) << 32;
  proposal_id += (uint64_t)rand++ << 16;
  proposal_id += (uint64_t)this->m_ddwNodeId;

  m_ddwProposalId = proposal_id;
  return proposal_id;
}
int Proposer::OnPrepareFailed()
{
  SPDLOG_DEBUG("prepare failed");
  m_oContext.CommitResult(false, GetInstanceID(), "");
  return 0;
}
int Proposer::OnAcceptFailed()
{
  SPDLOG_DEBUG("accept failed");
  m_oContext.CommitResult(false, GetInstanceID(), "");
  return 0;
}
int Proposer::Propose(const Proposal &oProposal)
{
  RespawnProposalId();
  m_oStage = Stage::Preparing;
  Message oMessage;
  this->m_oProposal = oProposal;
  oMessage.set_type(Message_Type::Message_Type_PREAPRE);
  oMessage.set_instance_id(GetInstanceID());
  auto &oPrepare = *oMessage.mutable_prepare();
  oPrepare.set_proposal_id(this->GetProposalId());

  this->m_oBallot.ResetBallot(3);
  this->Broadcast(BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_ACCEPTER, BroadcastType::BROAD_CAST_TYPE_ALL, oMessage);
  return 0;
}
int Proposer::Accept(const Proposal &oProposal)
{
  Message oMessage;
  oMessage.set_type(Message_Type::Message_Type_ACCEPT);
  oMessage.set_instance_id(GetInstanceID());
  oMessage.mutable_accept()->mutable_proposal()->CopyFrom(oProposal);
  oMessage.mutable_accept()->mutable_proposal()->set_id(this->GetProposalId());
  this->Broadcast(BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_ACCEPTER, BroadcastType::BROAD_CAST_TYPE_ALL, oMessage);
  return 0;
}
int Proposer::Learn(const Proposal &oProposal)
{
  Message oMessage;
  oMessage.set_type(Message_Type::Message_Type_LEARN);
  oMessage.set_instance_id(GetInstanceID());
  oMessage.mutable_accept()->mutable_proposal()->CopyFrom(oProposal);
  oMessage.mutable_accept()->mutable_proposal()->set_id(this->GetProposalId());
  this->Broadcast(BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_LEARNER, BroadcastType::BROAD_CAST_TYPE_ALL, oMessage);
  return 0;
}
int Proposer::OnPromised(const Message &oMessage)
{
  if (m_oStage != Stage::Preparing)
  {
    SPDLOG_DEBUG("Not preparing. skip promise {}", oMessage.ShortDebugString());
    return 0;
  }
  if (oMessage.promised().has_accepted_proposal())
  {
    m_oBallot.VoteUp(oMessage.from_node_id(), oMessage.promised().accepted_proposal());
  }
  else
  {
    m_oBallot.VoteUp(oMessage.from_node_id());
  }
  SPDLOG_DEBUG("VoteUpFrom: {} up:{} down:{} {}", oMessage.from_node_id(), m_oBallot.GetUpCount(), m_oBallot.GetDownCount(), oMessage.ShortDebugString());

  if (m_oBallot.IsMajorityUp())
  {
    SPDLOG_DEBUG("MajorityUp {} broadcast accept", m_oBallot.GetUpCount());
    m_oStage = Stage::Accepting;
    m_oProposal = m_oBallot.GetChosenProposal(this->m_oProposal);
    m_oBallot.ResetBallot(3);
    this->Accept(m_oProposal);
  }
  return 0;
}
int Proposer::OnRejectPromise(const Message &oMessage)
{
  if (m_oStage != Stage::Preparing)
  {
    SPDLOG_DEBUG("Not preparing. skip reject promise {}", oMessage.ShortDebugString());
    return 0;
  }
  m_oBallot.VoteDown(oMessage.from_node_id(), oMessage.reject_promise().promised_proposal_id());
  SPDLOG_DEBUG("VoteDownFrom: {} up:{} down:{}", oMessage.from_node_id(), m_oBallot.GetUpCount(), m_oBallot.GetDownCount());

  if (m_oBallot.IsMajorityDown())
  {
    m_oStage = Stage::Idle;
    this->OnPrepareFailed();
  }
  return 0;
}
int Proposer::OnAccepted(const Message &oMessage)
{
  if (m_oStage != Stage::Accepting)
  {
    SPDLOG_DEBUG("Not accepting. skip accepted {}", oMessage.ShortDebugString());
    return 0;
  }
  m_oBallot.VoteUp(oMessage.from_node_id());
  SPDLOG_DEBUG("Accepted VoteUpFrom: {} up:{} down:{}", oMessage.from_node_id(), m_oBallot.GetUpCount(), m_oBallot.GetDownCount());

  if (m_oBallot.IsMajorityUp())
  {
    SPDLOG_DEBUG("MajorityUp {} broadcast learn", m_oBallot.GetUpCount());
    m_oStage = Stage::Idle;
    this->Learn(m_oProposal);
  }
  return 0;
}
int Proposer::OnRejectAccept(const Message &oMessage)
{
  if (m_oStage != Stage::Accepting)
  {
    SPDLOG_DEBUG("Not accepting. skip reject accept {}", oMessage.ShortDebugString());
    return 0;
  }
  m_oBallot.VoteDown(oMessage.from_node_id(), 0);
  SPDLOG_DEBUG("Accepted VoteDownFrom: {} up:{} down:{}", oMessage.from_node_id(), m_oBallot.GetUpCount(), m_oBallot.GetDownCount());

  if (m_oBallot.IsMajorityDown())
  {
    m_oStage = Stage::Idle;
    this->OnAcceptFailed();
  }
  return 0;
}
int Proposer::OnReceiveMessage(const Message &oMessage)
{
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_PROMISED:
    this->OnPromised(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_PREOMISE:
    this->OnRejectPromise(oMessage);
    break;
  case Message_Type::Message_Type_ACCEPTED:
    this->OnAccepted(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_ACCEPT:
    this->OnRejectAccept(oMessage);
    break;
  }
  return 0;
}
void Proposer::InitForNewInstance()
{
}
}; // namespace paxoslib::role