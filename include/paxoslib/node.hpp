#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include "paxoslib/statemachine.hpp"
namespace paxoslib
{
namespace config
{
class Config;
};
class NodeImpl;
class Node
{
public:
  Node(const paxoslib::config::Config &oConfig);
  int Init();
  uint16_t GetNodeId() const;
  int Propose(uint32_t iGroupIndex, const std::string &, uint64_t &ddwInstanceId);
  void AddStateMachine(StateMachine *pSM);
  ~Node();

private:
  NodeImpl *m_pImpl;
};
}; // namespace paxoslib