#pragma once
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <map>
#include <set>
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/util/lfqueue.hpp"
#include "paxoslib/util/epoll.hpp"
namespace paxoslib
{
};

namespace paxoslib::network
{
class Channel;
class Peer;

class Network : public std::enable_shared_from_this<Network>
{
public:
  Network(const config::Config &);
  void OnReceivePeerMessage(uint16_t peer_id, std::unique_ptr<char[]> pBuffer, uint32_t size);

  void AddPeer(std::shared_ptr<Peer> pPeer);
  void AddChannel(std::shared_ptr<Channel> pChannel);
  void RemoveChannel(Channel *pChannel);
  void StartListner();
  void MakeChannelForPeer(uint16_t peer_id, const std::string &strIp, const int port);
  void SendMessageToPeer(uint16_t peer_id, std::unique_ptr<char[]> pBuffer, uint32_t size);
  uint32_t CountPeerChannel(uint16_t peer_id);
  uint16_t GetNodeId() const;
  ~Network();

private:
  std::shared_ptr<Peer> GetPeerById(uint16_t peer_id);
  std::shared_ptr<Channel> GetChannelByPeerId(uint16_t peer_id);
  std::shared_ptr<Channel> GetChannelByFd(int fd);
  enum class EventType
  {
    ChannelWriteable = 1,
    ChannelReadable = 2,
    ChannelEnqueueMessage = 3,
    PeerConnect = 4,
    PeerDisconnect = 5,
    MakePeerChannel = 6
  };
  struct Event
  {
    int fd;
    EventType type;
    uint64_t time;
    uint16_t peer_id;
  };
  void NetworkEventLoop();
  void EnqueueEvent(const Event &oEvent, bool bNoticeEventLoop);
  std::shared_ptr<std::vector<std::shared_ptr<Channel>>> m_pVecChannel;
  std::shared_ptr<std::vector<std::shared_ptr<Peer>>> m_pVecPeer;
  std::atomic<uint32_t> *m_pPeerChannelLastConnectTime;
  std::mutex m_mutexVecChannel;
  std::mutex m_mutexVecPeer;
  int m_event_fd;
  int m_listen_fd;
  uint64_t m_node_id;
  config::Config m_oConfig;
  util::LFQueue<Event> m_oEventQueue;
  util::EPoll m_epoll;
};
}; // namespace paxoslib::network