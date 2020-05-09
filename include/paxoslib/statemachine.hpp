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
  virtual int Execute(uint32_t iGroupIndex, uint64_t instance_id, const std::string &strChosenValue) = 0;
  virtual int ExecuteForCheckpointReplay(uint32_t iGroupIndex, uint64_t instance_id, const std::string &strChosenValue) = 0;
  virtual uint64_t GetCheckpointInstanceID(uint32_t iGroupIndex) = 0;
  virtual int LockCheckpointState(uint32_t iGroupIndex) = 0;
  virtual void UnlockCheckpointState(uint32_t iGroupIndex) = 0;
  virtual int LoadCheckpointState(uint32_t iGroupIndex, const std::string &strPath, const std::vector<std::string> &vecFiles) = 0;
  virtual int GetCheckpointState(uint32_t iGroupIndex, std::string &strPath, std::vector<std::string> &vecFiles) = 0;
};
}; // namespace paxoslib