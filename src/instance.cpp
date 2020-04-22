#include <vector>
#include <memory>
#include <iostream>
#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/instance.hpp"
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/role/learner.hpp"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "sys/eventfd.h"

namespace paxoslib
{

InstanceImpl::InstanceImpl(Instance *pInstance, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork) : m_pNetwork(pNetwork), m_oProposer(this), m_oAccepter(this), m_oLearner(this)
{
  m_event_fd = eventfd(0, EFD_SEMAPHORE);
  m_ddwNodeId = oConfig.node_id();
  for (const auto &oPeer : oConfig.peers())
  {
    if (oPeer.id() != oConfig.node_id())
    {
      auto pPeer = std::make_shared<network::Peer>(oPeer.id(), this, pNetwork);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_ACCEPTER);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_PROPOSER);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_LEARNER);
      pNetwork->AddPeer(pPeer);
      pNetwork->MakeChannelForPeer(oPeer.id(), oPeer.ip(), oPeer.port());
      m_vecPeers.push_back(pPeer);
    }
  }
  auto pPeer = std::make_shared<network::Peer>(m_ddwNodeId, this, pNetwork);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_ACCEPTER);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_PROPOSER);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_LEARNER);
  pNetwork->AddPeer(pPeer);
  m_vecPeers.push_back(pPeer);
  auto pSelfChannel = std::make_shared<network::ChannelSelf>(pNetwork, -1, m_ddwNodeId);
  pNetwork->AddChannel(pSelfChannel);
}
int InstanceImpl::OnMessage(const Message &oMessage)
{
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_PROMISED:
    m_oProposer.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_PREOMISE:
    m_oProposer.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_ACCEPTED:
    m_oProposer.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_ACCEPT:
    m_oProposer.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_ACCEPT:
    m_oAccepter.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_PREAPRE:
    m_oAccepter.OnMessage(oMessage);
    break;
  case Message_Type::Message_Type_LEARN:
    m_oLearner.OnMessage(oMessage);
    break;
  }
}
void InstanceImpl::NewInstance()
{
}
uint64_t InstanceImpl::GetInstanceId() const
{
  return this->m_ddwInstanceId;
}
const Proposal &InstanceImpl::GetProposal() const
{
  return this->m_oProposal;
}
uint16_t InstanceImpl::GetNodeId() const
{
  return this->m_ddwNodeId;
}
int InstanceImpl::Propose(const std::string &value)
{
  *m_oProposal.mutable_value() = value;
  m_ddwInstanceId = 1;
  m_oProposer.Propose(m_oProposal);
  return WaitResult();
}
std::vector<std::shared_ptr<network::Peer>> InstanceImpl::GetPeers() const
{
  return m_vecPeers;
}

int InstanceImpl::WaitResult()
{
  uint64_t value;
  read(m_event_fd, &value, sizeof(value));
  return 0;
}
int InstanceImpl::SubmitResult()
{
  uint64_t value = 1;
  write(m_event_fd, &value, sizeof(value));
  return 0;
}

Instance::Instance(const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork)
{
  pImpl = new InstanceImpl(this, oConfig, pNetwork);
}
uint64_t Instance::GetInstanceId() const
{
  return this->pImpl->GetInstanceId();
}
std::vector<std::shared_ptr<network::Peer>> Instance::GetPeers() const
{
  return this->pImpl->GetPeers();
}
const Proposal &Instance::GetProposal() const { return this->pImpl->GetProposal(); }
uint16_t Instance::GetNodeId() const { return this->pImpl->GetNodeId(); }
int Instance::Propose(const std::string &value)
{
  return this->pImpl->Propose(value);
}
Instance::~Instance()
{
  if (this->pImpl)
  {
    delete pImpl;
  }
}
}; // namespace paxoslib