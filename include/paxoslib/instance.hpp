#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
namespace paxoslib
{
class Message;
class StateMachineMgr;
namespace config
{
class Config;
};
namespace network
{
class Network;
class Peer;
} // namespace network
class Proposal;
class InstanceImpl;
namespace persistence
{
class Storage;
};
class Instance
{
public:
  Instance(const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork, std::unique_ptr<paxoslib::persistence::Storage> pStorage, std::shared_ptr<StateMachineMgr> pStateMachineMgr);
  uint64_t GetInstanceId() const;
  uint16_t GetNodeId() const;
  int Propose(const std::string &);
  std::vector<std::shared_ptr<network::Peer>> GetPeers() const;
  ~Instance();

private:
  InstanceImpl *pImpl;
};
}; // namespace paxoslib