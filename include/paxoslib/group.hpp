#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/eventfd.h>
#include "paxoslib/context.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/common.pb.h"
#include "paxoslib/persistence/storage.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/statemachine.hpp"
#include "paxoslib/statemachinemgr.hpp"
#include "paxoslib/eventloop/eventreceiver.hpp"
#include "paxoslib/eventloop/eventloop.hpp"
namespace paxoslib
{
class InstanceImpl;
class NodeImpl;
class Group
{
public:
  Group(uint32_t GroupSize);
  InstanceImpl *MakeInstance(uint32_t iGroupIndex, NodeImpl *pNode, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork, std::unique_ptr<paxoslib::persistence::Storage> pStorage, std::shared_ptr<StateMachineMgr> pStateMachineMgr);
  int DispatchPackage(uint32_t iGroupIndex, char *pBuffer, uint32_t size);
  InstanceImpl *GetInstance(uint32_t iGroupIndex);
  uint32_t GetGroupSize() const;
  int Propose(uint32_t iGroupIndex, const std::string &strValue, uint64_t &ddwInstanceId);
  ~Group();

private:
  int m_iGroupSize;
  std::vector<InstanceImpl *> m_vecInstances;
};
}; // namespace paxoslib