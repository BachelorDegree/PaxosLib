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
namespace paxoslib::network
{
class Network;
class ReceiveEventListener
{
public:
  virtual void OnMessage(const Message &oMessage) = 0;
};
class Peer
{
public:
  struct SendQueueItem
  {
    Message message;
  };
  struct ReceiveQueueItem
  {
    Message message;
  };
  Peer(uint64_t peer_id, ReceiveEventListener *pEventListner, Network *pNetwork);
  uint32_t GetPeerID() const;
  const std::set<RoleType> &GetRoleTypes() const;
  void AddRoleType(RoleType);
  uint64_t GetRoleTypesMask() const;
  void ReceiveEventWorker();
  void EnqueueSendMessage(const Message &oMessage);
  void EnqueueReceiveMessage(const Message &oMessage);
  void SendWorker();

private:
  void EmitFd(int *fds);
  void WaitFd(int *fds);
  int m_fd;
  int m_receive_fd[2];
  uint64_t m_peer_id;
  std::queue<SendQueueItem> m_SendQueue;
  std::queue<ReceiveQueueItem> m_ReceiveQueue;
  std::set<RoleType> m_setRoleTypes;
  ReceiveEventListener *m_pEventListner;
  Network *m_pNetwork;
  std::thread m_oReceiveEventWorkerThread;
  std::thread m_oSendWorkerThread;
};

}; // namespace paxoslib::network