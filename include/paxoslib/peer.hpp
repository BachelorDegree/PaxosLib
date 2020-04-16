#pragma once
#include <queue>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <set>
#include <thread>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/network.pb.h"
#include "paxoslib/proto/role.pb.h"
#include "paxoslib/util/lfqueue.hpp"
namespace paxoslib::network
{
class Network;
class ReceiveEventListener
{
public:
  virtual void OnMessage(const Message &oMessage) = 0;
  virtual ~ReceiveEventListener(){};
};
class Peer
{
public:
  struct ReceiveQueueItem
  {
    Message message;
  };
  Peer(uint64_t peer_id, ReceiveEventListener *pEventListner, std::shared_ptr<Network> pNetwork);
  uint32_t GetPeerID() const;
  const std::set<RoleType> &GetRoleTypes() const;
  void AddRoleType(RoleType);
  uint64_t GetRoleTypesMask() const;
  void ReceiveEventWorker();
  void SendMessage(const Message &oMessage);
  void EnqueueReceiveMessage(const Message &oMessage);

private:
  void EmitEvent(int fd);
  void WaitEvent(int fd);
  int m_fd;
  int m_event_fd;
  uint64_t m_peer_id;
  util::LFQueue<ReceiveQueueItem> m_ReceiveQueue;
  std::set<RoleType> m_setRoleTypes;
  ReceiveEventListener *m_pEventListner;
  std::shared_ptr<Network> m_pNetwork;
  std::thread m_oReceiveEventWorkerThread;
  std::thread m_oSendWorkerThread;
};

}; // namespace paxoslib::network