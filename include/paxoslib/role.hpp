#pragma once
#include <vector>
#include <set>
#include <mutex>
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/network.pb.h"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/common.pb.h"
namespace paxoslib
{
class InstanceImpl;
namespace role
{
class Role
{
public:
  Role(InstanceImpl *pInstance);
  void Broadcast(BroadcastReceiverType oReceiverType, BroadcastType oBroadcastType, const Message &oMessage);
  void SendMessageTo(uint32_t dwNodeId, const Message &oMessage);
  int OnMessage(const Message &oMessage);
  uint64_t GetInstanceID() const;
  void SetInstanceID(uint64_t);
  void NewInstance();

protected:
  virtual void InitForNewInstance() = 0;
  virtual int OnReceiveMessage(const Message &oMessage) = 0;
  InstanceImpl *m_pInstance;

private:
  std::mutex m_oMessageMutex;
  uint64_t m_ddwInstranceID;
};
}; // namespace role
}; // namespace paxoslib
