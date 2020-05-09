#include <vector>
#include <memory>
#include <iostream>
#include <spdlog/spdlog.h>

#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/node.hpp"
#include "paxoslib/nodeimpl.hpp"

namespace paxoslib
{
NodeImpl::NodeImpl(const paxoslib::config::Config &oConfig) : m_oConfig(oConfig), m_oGroup(10)
{
  m_pNetwork = std::make_shared<network::Network>(m_oConfig);
  m_pStateMachineMgr = std::make_shared<StateMachineMgr>();
}
void NodeImpl::AddStateMachine(StateMachine *pSM)
{
  this->m_pStateMachineMgr->AddStateMachine(pSM);
}
std::vector<std::shared_ptr<network::Peer>> NodeImpl::GetPeers() const
{
  return m_vecPeers;
}
int NodeImpl::Init()
{
  //init network
  m_ddwNodeId = m_oConfig.node_id();
  for (const auto &oPeer : m_oConfig.peers())
  {
    if (oPeer.id() != m_oConfig.node_id())
    {
      auto pPeer = std::make_shared<network::Peer>(oPeer.id(), this, m_pNetwork);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_ACCEPTER);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_PROPOSER);
      pPeer->AddRoleType(RoleType::ROLE_TYPE_LEARNER);
      m_pNetwork->AddPeer(pPeer);
      m_pNetwork->MakeChannelForPeer(oPeer.id(), oPeer.ip(), oPeer.port());
      m_vecPeers.push_back(pPeer);
    }
  }
  auto pPeer = std::make_shared<network::Peer>(m_ddwNodeId, this, m_pNetwork);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_ACCEPTER);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_PROPOSER);
  pPeer->AddRoleType(RoleType::ROLE_TYPE_LEARNER);
  m_pNetwork->AddPeer(pPeer);
  m_vecPeers.push_back(pPeer);
  auto pSelfChannel = std::make_shared<network::ChannelSelf>(m_pNetwork, -1, m_ddwNodeId);
  m_pNetwork->AddChannel(pSelfChannel);
  //init group
  for (int i = 0; i < this->m_oGroup.GetGroupSize(); i++)
  {
    auto pStorage = new paxoslib::persistence::StorageLeveldb{m_oConfig.paxoslog_dir() + std::to_string(i)};
    auto pInstnace = m_oGroup.MakeInstance(i, this, m_oConfig, m_pNetwork, std::unique_ptr<paxoslib::persistence::StorageLeveldb>(pStorage), m_pStateMachineMgr);
    if (pInstnace == nullptr)
    {
      return 2;
    }
  }
  return 0;
}
int NodeImpl::OnPackage(char *pBuffer, uint32_t size)
{
  uint32_t iGroupIndex;
  if (size < sizeof(uint32_t))
  {
    return -1;
  }
  memcpy(&iGroupIndex, pBuffer, sizeof(iGroupIndex));
  iGroupIndex = ntohl(iGroupIndex);
  return m_oGroup.DispatchPackage(iGroupIndex, pBuffer, size);
}
uint16_t NodeImpl::GetNodeId() const { return m_ddwNodeId; }
int NodeImpl::Propose(uint32_t iGroupIndex, const std::string &strValue, uint64_t &ddwInstanceId)
{
  return m_oGroup.Propose(iGroupIndex, strValue, ddwInstanceId);
}
NodeImpl::~NodeImpl()
{
}

Node::Node(const paxoslib::config::Config &oConfig)
{
  this->m_pImpl = new NodeImpl(oConfig);
}
int Node::Init()
{
  return this->m_pImpl->Init();
}
uint16_t Node::GetNodeId() const
{
  return this->m_pImpl->GetNodeId();
}
int Node::Propose(uint32_t iGroupIndex, const std::string &strValue, uint64_t &ddwInstanceId)
{
  return this->m_pImpl->Propose(iGroupIndex, strValue, ddwInstanceId);
}
void Node::AddStateMachine(StateMachine *pSM)
{
  this->m_pImpl->AddStateMachine(pSM);
}
Node::~Node()
{
  if (this->m_pImpl)
  {
    delete this->m_pImpl;
  }
}
}; // namespace paxoslib