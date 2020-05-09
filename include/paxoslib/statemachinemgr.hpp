#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "paxoslib/statemachine.hpp"
namespace paxoslib
{
class StateMachineMgr
{
public:
  StateMachineMgr();
  ~StateMachineMgr();
  void AddStateMachine(StateMachine *pSM);
  int Execute(uint32_t iGroupIndex, int smid, uint64_t instance_id, const std::string &strChosenValue);
  int ExecuteForSM(uint32_t iGroupIndex, StateMachine *pSM, uint64_t instance_id, const std::string &strChosenValue);
  int GetCheckpointMinInstanceID(uint32_t iGroupIndex, uint64_t &instance_id) const;
  int ExecuteForReplay(uint32_t iGroupIndex, int smid, uint64_t instance_id, const std::string &strChosenValue);

private:
  std::vector<StateMachine *> m_vecpSM;
};
}; // namespace paxoslib