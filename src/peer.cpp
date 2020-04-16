#include <thread>
#include "paxoslib/peer.hpp"
#include "paxoslib/network.hpp"
#include "sys/eventfd.h"
namespace paxoslib::network
{
Peer::Peer(uint64_t peer_id, ReceiveEventListener *pEventListner, std::shared_ptr<Network> pNetwork)
{
  m_pEventListner = pEventListner;
  m_peer_id = peer_id;
  m_pNetwork = pNetwork;
  m_event_fd = eventfd(0, EFD_SEMAPHORE);
  m_oReceiveEventWorkerThread = std::thread([&]() {
    this->ReceiveEventWorker();
  });
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
void Peer::ReceiveEventWorker()
{
  ReceiveQueueItem oItem;
  while (true)
  {
    WaitEvent(m_event_fd);
    while (m_ReceiveQueue.pop_front(oItem))
    {
      m_pEventListner->OnMessage(oItem.message);
    }
  }
}
void Peer::SendMessage(const Message &oMessage)
{
  this->m_pNetwork->SendMessageToPeer(m_peer_id, oMessage);
}
void Peer::EnqueueReceiveMessage(const Message &oMessage)
{
  ReceiveQueueItem oItem;
  oItem.message = oMessage;
  m_ReceiveQueue.push_back(std::move(oItem));
  EmitEvent(m_event_fd);
}
void Peer::AddRoleType(RoleType oType)
{
  this->m_setRoleTypes.insert(oType);
}
void Peer::EmitEvent(int fd)
{
  uint64_t a;
  a = 1;
  write(fd, &a, sizeof(a));
}
void Peer::WaitEvent(int fd)
{
  uint64_t a;
  read(fd, &a, sizeof(a));
}
}; // namespace paxoslib::network