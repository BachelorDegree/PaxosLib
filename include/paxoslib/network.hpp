#pragma once
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <map>
#include <set>
#include "paxoslib/proto/config.pb.h"
namespace paxoslib
{
class Message;
};
namespace paxoslib::network
{
class Channel;
class Peer;

class Network
{
public:
  Network(const config::Config &);
  void OnChannelReceiveMessgae(Channel *pChannel, const char *pBuffer, uint32_t size);
  void AddPeer(Peer *);
  void ListenerThread();
  void MakeChannelForPeer(uint64_t peer_id, const std::string &strIp, const int port);
  void SendMessageToPeer(uint64_t peer_id, const Message &oMessage);

private:
  std::map<uint64_t, std::set<Channel *>> m_pMapPeerChannel;
  std::map<uint64_t, Peer *> m_pMapPeer;
  int m_listen_fd;
  uint64_t m_node_id;
  std::thread m_listenThread;
  config::Config m_oConfig;
};
}; // namespace paxoslib::network