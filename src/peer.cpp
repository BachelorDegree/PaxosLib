#include <thread>
#include "paxoslib/peer.hpp"
#include "paxoslib/network.hpp"
#include "sys/eventfd.h"
#include "paxoslib/util/timetrace.hpp"
namespace paxoslib::network
{
Peer::Peer(uint16_t peer_id, eventloop::EventReceiver *pEventReceiver, eventloop::EventLoop *pEventLoop, std::shared_ptr<Network> pNetwork)
{
  m_pEventReceiver = pEventReceiver;
  m_peer_id = peer_id;
  m_pNetwork = pNetwork;
  m_pEventLoop = pEventLoop;
}
uint32_t Peer::GetPeerID() const
{
  return m_peer_id;
}
const std::set<RoleType> &Peer::GetRoleTypes() const
{
  return m_setRoleTypes;
}
uint64_t Peer::GetRoleTypesMask() const
{
  uint64_t ret = 0;
  for (auto a : m_setRoleTypes)
  {
    ret = ret | (1 << a);
  }
  return ret;
}
void Peer::SendMessage(const Message &oMessage)
{
  Trace::Mark(oMessage.id(), "Peer::SendMessage");
  this->m_pNetwork->SendMessageToPeer(m_peer_id, oMessage);
}
void Peer::EnqueueReceiveMessage(const Message &oMessage)
{
  auto pMessage = new Message(oMessage);
  Trace::Mark(pMessage->id(), "Peer::EnqueueReceiveMessage");
  m_pEventLoop->AddEventTail(m_pEventReceiver, 1, pMessage);
}
void Peer::AddRoleType(RoleType oType)
{
  this->m_setRoleTypes.insert(oType);
}
}; // namespace paxoslib::network