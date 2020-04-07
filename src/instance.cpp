#include <vector>
#include <memory>
#include <iostream>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/instance.hpp"
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/role/learner.hpp"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
namespace paxoslib
{
class Instance::InstanceImpl : public network::ReceiveEventListener
{

public:
  InstanceImpl(Instance *pInstance, const paxoslib::config::Config &oConfig, network::Network *pNetwork) : m_pNetwork(pNetwork), m_oProposer(pInstance), m_oAccepter(pInstance)
  {
    ddwNodeId = oConfig.node_id();
    for (const auto &oPeer : oConfig.peers())
    {
      if (oPeer.id() != oConfig.node_id())
      {
        auto pPeer = std::make_shared<network::Peer>(oPeer.id(), this, pNetwork);
        pPeer->AddRoleType(RoleType::ROLE_TYPE_ACCEPTER);
        pPeer->AddRoleType(RoleType::ROLE_TYPE_PROPOSER);
        pNetwork->AddPeer(pPeer.get());
        pNetwork->MakeChannelForPeer(oPeer.id(), oPeer.ip(), oPeer.port());
        m_vecPeers.push_back(pPeer);
      }
    }
  }
  virtual void OnMessage(const Message &oMessage)
  {
    if (oMessage.to_node_id() == 1)
    {
      std::cout << "ReceiveMessage from " << oMessage.from_node_id() << " data:" << oMessage.ShortDebugString() << std::endl;
    }
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
    }
  }
  void NewInstance()
  {
  }
  uint64_t GetInstanceId() const
  {
    return this->ddwInstanceId;
  }
  const Proposal &GetProposal() const
  {
    return this->m_oProposal;
  }
  uint64_t GetNodeId() const
  {
    return this->ddwNodeId;
  }
  int Propose(const std::string &value)
  {
    *m_oProposal.mutable_value() = value;
    ddwInstanceId = 1;
    m_oProposer.Propose(m_oProposal);
  }
  std::vector<network::Peer *> GetPeers() const
  {
    std::vector<network::Peer *> ret;
    for (auto pPeer : m_vecPeers)
    {
      ret.push_back(pPeer.get());
    }
    return ret;
  }

private:
  uint64_t ddwInstanceId;
  uint64_t ddwNodeId;
  Proposal m_oProposal;
  std::vector<std::shared_ptr<network::Peer>> m_vecPeers;
  network::Network *m_pNetwork;
  role::Proposer m_oProposer;
  role::Accepter m_oAccepter;
};
Instance::Instance(const paxoslib::config::Config &oConfig, network::Network *pNetwork)
{
  pImpl = new InstanceImpl(this, oConfig, pNetwork);
}
uint64_t Instance::GetInstanceId() const
{
  return this->pImpl->GetInstanceId();
}
std::vector<network::Peer *> Instance::GetPeers() const
{
  return this->pImpl->GetPeers();
}
const Proposal &Instance::GetProposal() const { return this->pImpl->GetProposal(); }
uint64_t Instance::GetNodeId() const { return this->pImpl->GetNodeId(); }
int Instance::Propose(const std::string &value)
{
  return this->pImpl->Propose(value);
}
}; // namespace paxoslib