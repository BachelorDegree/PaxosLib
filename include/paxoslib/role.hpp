#pragma once
#include <vector>
#include <set>
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/network.pb.h"
#include "paxoslib/proto/message.pb.h"

namespace paxoslib
{
class Instance;
namespace role
{
class Role : public paxoslib::network::ReceiveEventListener
{
public:
  Role(Instance *pInstance);
  void Broadcast(BroadcastReceiverType oReceiverType, BroadcastType oBroadcastType, const Message &oMessage);
  void SendMessageTo(uint32_t dwNodeId, const Message &oMessage);
  virtual void OnMessage(const Message &oMessage) = 0;

private:
  Instance *m_pInstance;
};
}; // namespace role
}; // namespace paxoslib
