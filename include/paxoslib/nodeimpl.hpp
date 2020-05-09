#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/persistence/storageleveldb.hpp"
#include "paxoslib/statemachinemgr.hpp"
#include "paxoslib/eventloop/eventloop.hpp"
#include "paxoslib/packagereceiver.hpp"
#include "paxoslib/group.hpp"
namespace paxoslib
{
class NodeImpl : public network::PackageReceiver
{
public:
  ~NodeImpl();
  NodeImpl(const paxoslib::config::Config &oConfig);
  int Init();
  uint16_t GetNodeId() const;
  int Propose(uint32_t iGroupIndex, const std::string &, uint64_t &ddwInstanceId);
  void AddStateMachine(StateMachine *pSM);
  virtual int OnPackage(char *pBuffer, uint32_t size);
  std::vector<std::shared_ptr<network::Peer>> GetPeers() const;

private:
  config::Config m_oConfig;
  uint16_t m_ddwNodeId;
  std::vector<std::shared_ptr<network::Peer>> m_vecPeers;
  std::shared_ptr<network::Network> m_pNetwork;
  std::shared_ptr<StateMachineMgr> m_pStateMachineMgr;
  Group m_oGroup;
};
}; // namespace paxoslib