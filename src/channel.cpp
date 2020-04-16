#pragma once
#include <iostream>
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/peer.hpp"
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) (((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif
namespace paxoslib::network
{
Channel::Channel(std::weak_ptr<Network> pNetwork, int fd) : m_fd(fd)
{
  m_pNetwork = pNetwork;
  m_bHealthy = false;
  m_ReceiveState.state = 0;
  m_SendState.state = 0;
}
int Channel::GetFd() const
{
  return m_fd;
}
Channel::~Channel()
{
}
void Channel::OnReadable()
{
  while (true)
  {
    switch (m_ReceiveState.state)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    {
      ssize_t size = recv(m_fd, &m_ReceiveState.size + m_ReceiveState.state, 4 - m_ReceiveState.state, MSG_DONTWAIT);
      if (size <= 0)
      {
        return;
      }
      m_ReceiveState.state += size;
      break;
    }
    case 4:
    {
      m_ReceiveState.size = ntohl(m_ReceiveState.size);
      m_ReceiveState.pBuffer = std::unique_ptr<char[]>(new char[m_ReceiveState.size]);
      m_ReceiveState.read = 0;
      m_ReceiveState.state = 5;
      break;
    }
    case 5:
    {
      ssize_t to_read = 4096;
      if (to_read > m_ReceiveState.size - m_ReceiveState.read)
      {
        to_read = m_ReceiveState.size - m_ReceiveState.read;
      }
      ssize_t size = recv(m_fd, m_ReceiveState.pBuffer.get() + m_ReceiveState.read, to_read, MSG_DONTWAIT);
      if (size <= 0)
      {
        return;
      }
      m_ReceiveState.read += size;
      if (m_ReceiveState.read >= m_ReceiveState.size)
      {
        m_ReceiveState.state = 6;
      }
      break;
    }
    case 6:
    {
      if (auto pNetwork = m_pNetwork.lock())
      {
        pNetwork->OnReceivePeerMessage(this->GetPeerId(), std::move(m_ReceiveState.pBuffer), m_ReceiveState.size);
      }
      m_ReceiveState.state = 0;
      return;
    }
    }
  }
};
void Channel::EnqueueSendMessage(std::unique_ptr<char[]> pBuffer, uint32_t size)
{
  QueueItem oItem;
  oItem.pBuffer = std::move(pBuffer);
  oItem.size = size;
  std::vector<QueueItem> vec;
  m_SendQueue.push_back(std::move(oItem));
}
void Channel::OnDisconnect()
{
}
void Channel::OnWritableOrTaskArrive()
{
  while (true)
  {
    switch (m_SendState.state)
    {
    case 0:
    {
      QueueItem oItem;
      if (!m_SendQueue.pop_front(oItem))
      {
        return;
      }
      m_SendState.state = 1;
      m_SendState.pBuffer = std::move(oItem.pBuffer);
      m_SendState.size = oItem.size;
      m_SendState.write = 0;
      break;
    }
    case 1:
    case 2:
    case 3:
    case 4:
    {
      uint32_t n_size = htonl(m_SendState.size);
      ssize_t size = send(m_fd, &n_size + m_SendState.state - 1, 4 - m_SendState.state + 1, MSG_DONTWAIT);
      if (size <= 0)
      {
        return;
      }
      m_SendState.state += size;
      break;
    }
    case 5:
    {
      ssize_t to_write = 4096;
      if (to_write > m_SendState.size - m_SendState.write)
      {
        to_write = m_SendState.size - m_SendState.write;
      }
      ssize_t size = send(m_fd, m_SendState.pBuffer.get() + m_SendState.write, to_write, MSG_DONTWAIT);
      if (size <= 0)
      {
        return;
      }
      m_SendState.write += size;
      if (m_SendState.write >= m_SendState.size)
      {
        m_SendState.state = 6;
      }
      break;
    }
    case 6:
    {
      m_SendState.state = 0;
      m_SendState.pBuffer.release();
      break;
    }
    }
  }
}
ChannelIncoming::ChannelIncoming(std::weak_ptr<Network> pNetwork, int fd) : Channel(pNetwork, fd)
{
  m_iPrepareState = 0;
  this->OnReadable();
}
void ChannelIncoming::OnReadable()
{
  while (true)
  {
    switch (m_iPrepareState)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    {
      ssize_t size = recv(m_fd, &m_peer_id + m_iPrepareState, 8 - m_iPrepareState, MSG_DONTWAIT);
      if (size <= 0)
      {
        return;
      }
      m_iPrepareState += size;
      break;
    }
    case 8:
    {
      m_peer_id = ntohll(m_peer_id);
      m_iPrepareState = 9;
      break;
    }
    case 9:
      Channel::OnReadable();
      return;
    }
  }
}
uint64_t ChannelIncoming::GetPeerId() const
{
  if (m_iPrepareState != 9)
  {
    return UINT64_MAX;
  }
  return m_peer_id;
}
ChannelOutgoing::ChannelOutgoing(std::weak_ptr<Network> pNetwork, int fd, uint64_t peer_id) : Channel(pNetwork, fd)
{
  m_iPrepareState = 0;
  m_peer_id = peer_id;
  auto _pNetwork = pNetwork.lock();
  m_my_id = _pNetwork->GetNodeId();
  this->OnWritableOrTaskArrive();
}
void ChannelOutgoing::OnWritableOrTaskArrive()
{
  while (true)
  {
    switch (m_iPrepareState)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    {
      uint64_t peer_id_n = htonll(m_my_id);
      ssize_t size = send(m_fd, &peer_id_n + m_iPrepareState, 8 - m_iPrepareState, MSG_DONTWAIT);
      std::cout << size << " " << errno << std::endl;
      if (size <= 0)
      {
        return;
      }
      m_iPrepareState += size;
      break;
    }
    case 8:
    {
      m_iPrepareState = 9;
      break;
    }
    case 9:
      Channel::OnWritableOrTaskArrive();
      return;
    }
  }
}
uint64_t ChannelOutgoing::GetPeerId() const
{
  return m_peer_id;
}

}; // namespace paxoslib::network
