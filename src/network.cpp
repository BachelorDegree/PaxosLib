#pragma once
#include <iostream>
#include <mutex>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/config.pb.h"
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
std::mutex oMutex;
namespace paxoslib::network
{
Network::Network(const config::Config &oConfig)
{
  m_node_id = oConfig.node_id();
  m_oConfig = oConfig;
  ListenerThread();
}
void Network::SendMessageToPeer(uint64_t peer_id, const Message &oMessage)
{
  if (this->m_pMapPeerChannel[peer_id].size() == 0)
  {
    return;
  }
  auto pChannel = *this->m_pMapPeerChannel[peer_id].begin();
  uint32_t size = oMessage.ByteSizeLong();
  char *pBuffer = new char[size];
  oMessage.SerializeToArray(pBuffer, size);
  pChannel->EnqueueSendMessage(pBuffer, size);
}
void Network::OnChannelReceiveMessgae(Channel *pChannel, const char *pBuffer, uint32_t size)
{
  std::lock_guard<std::mutex> oGuard(oMutex);
  if (this->m_pMapPeer.find(pChannel->GetPeerId()) != this->m_pMapPeer.end())
  {
    Message oMessage;
    oMessage.ParseFromArray(pBuffer, size);
    this->m_pMapPeer.at(pChannel->GetPeerId())->EnqueueReceiveMessage(oMessage);
  }
}
void Network::AddPeer(Peer *pPeer)
{
  this->m_pMapPeer[pPeer->GetPeerID()] = pPeer;
}
void Network::ListenerThread()
{
  this->m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_oConfig.port());
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  int optval = 1;
  setsockopt(this->m_listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  bind(this->m_listen_fd, (struct sockaddr *)&addr, sizeof(addr));
  listen(this->m_listen_fd, 100);
  m_listenThread = std::thread([&]() {
    while (true)
    {
      sockaddr_in client_addr;
      socklen_t addr_len = 0;
      int connect_fd = accept(this->m_listen_fd, (struct sockaddr *)&client_addr, &addr_len);
      uint64_t peer_id = 0;
      read(connect_fd, &peer_id, sizeof(peer_id));
      peer_id = ntohll(peer_id);
      Channel *pChannel = new Channel(this, connect_fd, peer_id);
      std::cout << "Incoming peer " << peer_id << " " << connect_fd << std::endl;
      this->m_pMapPeerChannel[peer_id].insert(pChannel);
    }
  });
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
  std::cout << "connect peer " << peer_id << " " << iRet << " " << fd << std::endl;
  uint64_t n_peer_id = htonll(this->m_node_id);
  write(fd, &n_peer_id, sizeof(n_peer_id));
  Channel *pChannel = new Channel(this, fd, peer_id);
  this->m_pMapPeerChannel[peer_id].insert(pChannel);
}
}; // namespace paxoslib::network