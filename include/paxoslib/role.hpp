#pragma once
#include <vector>
#include <set>
#include <mutex>
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/network.pb.h"
#include "paxoslib/proto/message.pb.h"

namespace paxoslib
{
class InstanceImpl;
namespace role
{
class Role : public paxoslib::network::ReceiveEventListener
{
public:
  Role(InstanceImpl *pInstance);
  void Broadcast(BroadcastReceiverType oReceiverType, BroadcastType oBroadcastType, const Message &oMessage);
  void SendMessageTo(uint32_t dwNodeId, const Message &oMessage);
  int OnMessage(const Message &oMessage);

protected:
  virtual int OnReceiveMessage(const Message &oMessage) = 0;
  InstanceImpl *m_pInstance;

private:
  std::mutex m_oMessageMutex;
};
}; // namespace role
}; // namespace paxoslib
