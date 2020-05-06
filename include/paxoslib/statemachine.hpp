#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace paxoslib
{
class StateMachine
{
public:
  StateMachine(){};
  virtual ~StateMachine() {}
  virtual int GetID() = 0;
  virtual int Execute(uint64_t instance_id, const std::string &strChosenValue) = 0;
  virtual int ExecuteForCheckpointReplay(uint64_t instance_id, const std::string &strChosenValue) = 0;
  virtual uint64_t GetCheckpointInstanceID() = 0;
  virtual int LockCheckpointState() = 0;
  virtual void UnlockCheckpointState() = 0;
  virtual int LoadCheckpointState(const std::string &strPath, const std::vector<std::string> &vecFiles) = 0;
  virtual int GetCheckpointState(std::string &strPath, std::vector<std::string> &vecFiles) = 0;
};
}; // namespace paxoslib