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
  int Execute(int smid, uint64_t instance_id, const std::string &strChosenValue);
  int ExecuteForSM(StateMachine *pSM, uint64_t instance_id, const std::string &strChosenValue);
  int GetCheckpointMinInstanceID(uint64_t &instance_id) const;
  int ExecuteForReplay(int smid, uint64_t instance_id, const std::string &strChosenValue);

private:
  std::vector<StateMachine *> m_vecpSM;
};
}; // namespace paxoslib