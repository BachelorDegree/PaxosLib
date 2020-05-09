#pragma once
#include <string>
#include <cstdint>
#include <condition_variable>
namespace paxoslib
{
class Context
{
public:
  Context();
  void Reset(uint64_t id, const std::string &value);
  int WaitAndGetResult(uint64_t &id);
  void CommitResult(bool bSuccess, uint64_t id, const std::string &value);

private:
  bool IsChosenValue(uint64_t id, const std::string &value) const;
  bool bCommited;
  uint64_t m_ddwInstanceId;
  int m_iRet;
  std::string m_strValue;
  std::condition_variable m_cv;
  std::mutex m_mutex;
};
}; // namespace paxoslib