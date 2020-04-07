#include "paxoslib/role.hpp"
#include "paxoslib/instance.hpp"
namespace paxoslib::role
{
Role::Role(Instance *pInstance)
{
  m_pInstance = pInstance;
}
void Role::Broadcast(BroadcastReceiverType oReceiverType, BroadcastType oBroadcastType, const Message &oMessage)
{
  Message oNewMessage{oMessage};
  oNewMessage.set_from_node_id(this->m_pInstance->GetNodeId());
  const std::map<BroadcastReceiverType, std::set<RoleType>> mapReceiver{
      {BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_ACCEPTER, {RoleType::ROLE_TYPE_ACCEPTER}}, {BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_PROPOSER, {RoleType::ROLE_TYPE_PROPOSER}}, {BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_LEARNER, {RoleType::ROLE_TYPE_LEARNER}}};
  uint64_t mask = 0;
  for (auto a : mapReceiver.at(oReceiverType))
  {
    mask = mask | (1 << a);
  }
  if (oBroadcastType == BroadcastType::BROAD_CAST_TYPE_SELF_THEN_OTHERS)
  {
    oNewMessage.set_to_node_id(this->m_pInstance->GetNodeId());
    this->OnMessage(oNewMessage);
    //wait notify
  }
  for (auto pPeer : m_pInstance->GetPeers())
  {
    if (pPeer->GetRoleTypesMask() & mask)
    {
      oNewMessage.set_to_node_id(pPeer->GetPeerID());
      pPeer->EnqueueSendMessage(oNewMessage);
    }
  }
  if (oBroadcastType == BroadcastType::BROAD_CAST_TYPE_ALL)
  {
    oNewMessage.set_to_node_id(this->m_pInstance->GetNodeId());
    this->OnMessage(oNewMessage);
  }
}
void Role::SendMessageTo(uint32_t dwNodeId, const Message &oMessage)
{
  Message oNewMessage{oMessage};
  for (auto pPeer : m_pInstance->GetPeers())
  {
    if (pPeer->GetPeerID() == dwNodeId)
    {
      oNewMessage.set_from_node_id(this->m_pInstance->GetNodeId());
      oNewMessage.set_to_node_id(dwNodeId);
      pPeer->EnqueueSendMessage(oNewMessage);
    }
  }
}
}; // namespace paxoslib::role
