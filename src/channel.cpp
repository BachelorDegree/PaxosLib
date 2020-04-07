#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/peer.hpp"
namespace paxoslib::network
{
Channel::Channel(Network *pNetwork, int fd, uint64_t peer_id) : m_fd(fd), m_peer_id(peer_id)
{
  m_pNetwork = pNetwork;
  m_bHealthy = false;
  m_SendThread = std::thread([&]() {
    this->SendWorker();
  });
  m_ReceiveThread = std::thread([&]() {
    this->ReceiveWorker();
  });
}
bool Channel::IsChannelHealthy() const
{
  return m_bHealthy;
}
ssize_t Channel::Send(const void *__buf, size_t __n, int __flags)
{
  return send(m_fd, __buf, __n, __flags);
}
ssize_t Channel::Recv(void *__buf, size_t __n, int __flags)
{
  return recv(m_fd, __buf, __n, __flags);
}
void Channel::EnqueueSendMessage(char *pBuffer, uint32_t size)
{
  QueueItem oItem;
  oItem.pBuffer = pBuffer;
  oItem.size = size;
  m_SendQueue.push(oItem);
}
void Channel::SendWorker()
{
  while (true)
  {
    if (m_SendQueue.size() == 0)
    {
      poll(0, 0, 10);
      continue;
    }
    QueueItem oItem = m_SendQueue.front();
    m_SendQueue.pop();
    uint32_t size = htonl(oItem.size);
    send(m_fd, (void *)&size, sizeof(size), 0);
    send(m_fd, oItem.pBuffer, oItem.size, 0);
    // for (int i = 0; i < oItem.size; i++)
    // {
    //   std::cout << (int)oItem.pBuffer[i];
    // }
    // std::cout << "send " << oItem.size << " " << m_peer_id << std::endl;
    delete[] oItem.pBuffer;
  }
}
void Channel::ReceiveWorker()
{
  while (true)
  {
    uint32_t size;
    recv(m_fd, &size, sizeof(size), 0);
    size = ntohl(size);
    char *buffer = new char[size];
    recv(m_fd, buffer, size, 0);
    // for (int i = 0; i < size; i++)
    // {
    //   std::cout << (int)buffer[i];
    // }
    // std::cout << "receive " << size << " " << m_peer_id << std::endl;
    m_pNetwork->OnChannelReceiveMessgae(this, buffer, size);
  }
}
uint64_t Channel::GetPeerId() const
{
  return this->m_peer_id;
};
}; // namespace paxoslib::network
