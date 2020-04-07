#include <thread>
#include "paxoslib/peer.hpp"
#include "paxoslib/network.hpp"
namespace paxoslib::network
{
Peer::Peer(uint64_t peer_id, ReceiveEventListener *pEventListner, Network *pNetwork)
{
  m_pEventListner = pEventListner;
  m_peer_id = peer_id;
  m_pNetwork = pNetwork;
  pipe(m_receive_fd);
  m_oReceiveEventWorkerThread = std::thread([&]() {
    this->ReceiveEventWorker();
  });
  m_oSendWorkerThread = std::thread([&]() {
    this->SendWorker();
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
  while (true)
  {
    WaitFd(m_receive_fd);
    while (m_ReceiveQueue.empty() == false)
    {
      ReceiveQueueItem oItem = std::move(m_ReceiveQueue.front());
      m_pEventListner->OnMessage(oItem.message);
      m_ReceiveQueue.pop();
    }
  }
}
void Peer::EnqueueSendMessage(const Message &oMessage)
{
  SendQueueItem oItem;
  oItem.message = oMessage;
  if (oMessage.from_node_id() == 1)
  {
    std::cout << "send to " << this->m_peer_id << " message:" << oMessage.ShortDebugString() << std::endl;
  }
  m_SendQueue.push(std::move(oItem));
}
void Peer::EnqueueReceiveMessage(const Message &oMessage)
{
  ReceiveQueueItem oItem;
  oItem.message = oMessage;
  m_ReceiveQueue.push(std::move(oItem));
  EmitFd(m_receive_fd);
}
void Peer::AddRoleType(RoleType oType)
{
  this->m_setRoleTypes.insert(oType);
}
void Peer::SendWorker()
{
  while (true)
  {
    if (m_SendQueue.size() == 0)
    {
      poll(0, 0, 10);
      continue;
    }
    SendQueueItem oItem = std::move(m_SendQueue.front());
    m_SendQueue.pop();
    this->m_pNetwork->SendMessageToPeer(m_peer_id, oItem.message);
  }
}
void Peer::EmitFd(int *fds)
{
  char a;
  a = 1;
  write(fds[0], &a, sizeof(a));
}
void Peer::WaitFd(int *fds)
{
  char a;
  read(fds[1], &a, sizeof(a));
}
}; // namespace paxoslib::network