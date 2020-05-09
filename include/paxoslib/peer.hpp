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
#include "paxoslib/packagereceiver.hpp"
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
  Peer(uint16_t peer_id, network::PackageReceiver *pPackageReceiver, std::shared_ptr<Network> pNetwork);
  uint32_t GetPeerID() const;
  const std::set<RoleType> &GetRoleTypes() const;
  void AddRoleType(RoleType);
  uint64_t GetRoleTypesMask() const;
  void ReceiveEventWorker();
  void SendMessage(const Message &oMessage);
  int EnqueueReceiveMessage(std::unique_ptr<char[]> pBuffer, uint32_t size);

private:
  uint16_t m_peer_id;
  std::set<RoleType> m_setRoleTypes;
  network::PackageReceiver *m_pPackageReceiver;
  std::shared_ptr<Network> m_pNetwork;
};

}; // namespace paxoslib::network