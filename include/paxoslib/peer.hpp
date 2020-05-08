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
#include "paxoslib/eventloop/eventreceiver.hpp"
#include "paxoslib/eventloop/eventloop.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/network.pb.h"
#include "paxoslib/proto/role.pb.h"
namespace paxoslib::network
{
class Network;
class Peer
{
public:
  struct ReceiveQueueItem
  {
    Message message;
  };
  Peer(uint16_t peer_id, eventloop::EventReceiver *pEventReceiver, eventloop::EventLoop *pEventLoop, std::shared_ptr<Network> pNetwork);
  uint32_t GetPeerID() const;
  const std::set<RoleType> &GetRoleTypes() const;
  void AddRoleType(RoleType);
  uint64_t GetRoleTypesMask() const;
  void ReceiveEventWorker();
  void SendMessage(const Message &oMessage);
  void EnqueueReceiveMessage(const Message &oMessage);

private:
  uint16_t m_peer_id;
  std::set<RoleType> m_setRoleTypes;
  eventloop::EventReceiver *m_pEventReceiver;
  eventloop::EventLoop *m_pEventLoop;
  std::shared_ptr<Network> m_pNetwork;
  std::thread m_oReceiveEventWorkerThread;
  std::thread m_oSendWorkerThread;
};

}; // namespace paxoslib::network