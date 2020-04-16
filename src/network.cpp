#pragma once
#include <iostream>
#include <mutex>
#include <memory>
#include <atomic>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/peer.hpp"
#include "sys/eventfd.h"
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) (((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif
namespace paxoslib::network
{
Network::Network(const config::Config &oConfig) : m_epoll(1024)
{
  m_pVecChannel = std::make_shared<std::vector<std::shared_ptr<Channel>>>();
  m_pVecPeer = std::make_shared<std::vector<std::shared_ptr<Peer>>>();
  m_node_id = oConfig.node_id();
  m_oConfig = oConfig;
  m_event_fd = eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE);
  m_epoll.WatchReadable(m_event_fd);
  StartListner();
  auto thread = std::thread([&]() {
    this->EventLoop();
  });
  thread.detach();
}
std::shared_ptr<Peer> Network::GetPeerById(uint64_t peer_id)
{
  auto pVecPeer = std::atomic_load(&m_pVecPeer);
  for (auto pPeer : *pVecPeer)
  {
    if (pPeer->GetPeerID() == peer_id)
    {
      return pPeer;
    }
  }
  return {};
}
std::shared_ptr<Channel> Network::GetChannelByPeerId(uint64_t peer_id)
{
  auto pVecChannel = std::atomic_load(&m_pVecChannel);
  for (auto pChannel : *pVecChannel)
  {
    if (pChannel->GetPeerId() == peer_id)
    {
      return pChannel;
    }
  }
  return {};
}
std::shared_ptr<Channel> Network::GetChannelByFd(int fd)
{
  auto pVecChannel = std::atomic_load(&m_pVecChannel);
  for (auto pChannel : *pVecChannel)
  {
    if (pChannel->GetFd() == fd)
    {
      return pChannel;
    }
  }
  return {};
}
void Network::SendMessageToPeer(uint64_t peer_id, const Message &oMessage)
{
  if (auto pChannel = GetChannelByPeerId(peer_id))
  {
    uint32_t size = oMessage.ByteSizeLong();
    char *pBuffer = new char[size];
    oMessage.SerializeToArray(pBuffer, size);
    pChannel->EnqueueSendMessage(std::unique_ptr<char[]>(pBuffer), size);
    Event oEvent;
    oEvent.type = EventType::ChannelEnqueueMessage;
    oEvent.fd = pChannel->GetFd();
    EnqueueEvent(oEvent, true);
  }
}
uint64_t Network::GetNodeId() const
{
  return m_node_id;
}
void Network::OnReceivePeerMessage(uint64_t peer_id, std::unique_ptr<char[]> pBuffer, uint32_t size)
{
  if (auto pPeer = GetPeerById(peer_id))
  {
    Message oMessage;
    oMessage.ParseFromArray(pBuffer.get(), size);
    pPeer->EnqueueReceiveMessage(oMessage);
  }
}
void Network::AddPeer(std::shared_ptr<Peer> pPeer)
{
  std::lock_guard<std::mutex> oGuard{m_mutexVecPeer};
  auto pNewVec = std::make_shared<std::vector<std::shared_ptr<Peer>>>(*std::atomic_load(&m_pVecPeer));
  pNewVec->push_back(pPeer);
  std::atomic_store(&m_pVecPeer, pNewVec);
}
void Network::AddChannel(std::shared_ptr<Channel> pChannel)
{
  std::lock_guard<std::mutex> oGuard{m_mutexVecChannel};
  auto pNewVec = std::make_shared<std::vector<std::shared_ptr<Channel>>>(*std::atomic_load(&m_pVecChannel));
  pNewVec->push_back(pChannel);
  std::atomic_store(&m_pVecChannel, pNewVec);
}
void Network::StartListner()
{
  this->m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_oConfig.port());
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  int optval = 1;
  setsockopt(this->m_listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  bind(this->m_listen_fd, (struct sockaddr *)&addr, sizeof(addr));
  m_epoll.WatchDisconnect(this->m_listen_fd);
  m_epoll.WatchReadable(this->m_listen_fd);
  listen(this->m_listen_fd, 100);
}
void Network::MakeChannelForPeer(uint64_t peer_id, const std::string &strIp, const int port)
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, strIp.c_str(), &addr.sin_addr) <= 0)
  {
    return;
  }
  int iRet = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
  if (iRet != 0)
  {
    return;
  }
  m_epoll.WatchDisconnect(fd);
  m_epoll.WatchReadable(fd);
  m_epoll.WatchWritable(fd);
  std::cout << "connect peer " << peer_id << " " << iRet << " " << fd << std::endl;
  auto pChannel = std::make_shared<ChannelOutgoing>(shared_from_this(), fd, peer_id);
  this->AddChannel(pChannel);
}
template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}
void Network::EnqueueEvent(const Event &oEvent, bool bNoticeEventLoop)
{
  m_oEventQueue.push_back(oEvent);
  std::cout << "enqueue event " << as_integer(oEvent.type) << " " << oEvent.fd << std::endl;
  if (bNoticeEventLoop)
  {
    uint64_t signal = 1;
    write(m_event_fd, &signal, sizeof(signal));
  }
}
void Network::EventLoop()
{
  epoll_event *pEpollEvents = new epoll_event[20];
  while (true)
  {
    int iEventCnt = m_epoll.WatiEvents(pEpollEvents, 20, -1);
    for (int i = 0; i < iEventCnt; i++)
    {
      auto &oEpollEvent = pEpollEvents[i];
      auto fd = oEpollEvent.data.fd;
      if (fd == this->m_listen_fd)
      {
        if (oEpollEvent.events & POLL_IN)
        {
          this->EnqueueEvent({.fd = -1,
                              .type = EventType::PeerConnect},
                             false);
        }
      }
      else if (fd == this->m_event_fd)
      {
        uint64_t signal;
        read(this->m_event_fd, &signal, sizeof(signal));
      }
      else
      {
        if (oEpollEvent.events & POLL_IN)
        {
          this->EnqueueEvent({.fd = fd,
                              .type = EventType::ChannelReadable},
                             false);
        }
        if (oEpollEvent.events & POLL_OUT)
        {
          this->EnqueueEvent({.fd = fd,
                              .type = EventType::ChannelWriteable},
                             false);
        }
        if ((oEpollEvent.events & POLLHUP) || (oEpollEvent.events & EPOLLRDHUP))
        {
          this->EnqueueEvent({.fd = fd,
                              .type = EventType::PeerDisconnect},
                             false);
        }
      }
    }
    Event oEvent;
    while (m_oEventQueue.pop_front(oEvent))
    {
      switch (oEvent.type)
      {
      case EventType::PeerConnect:
      {
        sockaddr_in client_addr;
        socklen_t addr_len = 0;
        int connect_fd = accept(this->m_listen_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (connect_fd < 0)
        {
          continue;
        }
        m_epoll.WatchReadable(connect_fd);
        m_epoll.WatchWritable(connect_fd);
        m_epoll.WatchDisconnect(connect_fd);
        auto pChannel = std::make_shared<ChannelIncoming>(shared_from_this(), connect_fd);
        AddChannel(pChannel);
        break;
      }
      case EventType::PeerDisconnect:
      {
        if (auto pChannel = GetChannelByFd(oEvent.fd))
        {
          pChannel->OnDisconnect();
        }
        m_epoll.Unwatch(oEvent.fd);
        break;
      }
      case EventType::ChannelEnqueueMessage:
      case EventType::ChannelWriteable:
      {
        if (auto pChannel = GetChannelByFd(oEvent.fd))
        {
          pChannel->OnWritableOrTaskArrive();
        }
        break;
      }
      case EventType::ChannelReadable:
      {
        if (auto pChannel = GetChannelByFd(oEvent.fd))
        {
          pChannel->OnReadable();
        }
        break;
      }
      }
    }
  }
  delete[] pEpollEvents;
}
}; // namespace paxoslib::network