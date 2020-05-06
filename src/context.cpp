#include <spdlog/spdlog.h>
#include "paxoslib/context.hpp"
namespace paxoslib
{

Context::Context() {}
void Context::Reset(uint64_t id, const std::string &value)
{
  m_ddwInstanceId = id;
  m_strValue = value;
}
int Context::WaitAndGetResult()
{
  std::unique_lock<std::mutex> lk(m_mutex);
  m_cv.wait(lk);
  return m_iRet;
}

void Context::CommitResult(bool bSuccess, uint64_t id, const std::string &value)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  bool bResult = true;
  if (!bSuccess)
  {
    SPDLOG_ERROR("CommitResult Failed: bSuccess == false");
    bResult = false;
    m_iRet = -1;
  }
  if (bSuccess && !IsChosenValue(id, value))
  {
    SPDLOG_ERROR("CommitResult Failed: not chosen value {} {}", m_strValue, value);
    bResult = false;
    m_iRet = -2;
  }
  if (bResult)
  {
    m_iRet = 0;
  }
  m_cv.notify_one();
}
bool Context::IsChosenValue(uint64_t id, const std::string &value) const
{
  return id == m_ddwInstanceId && value == m_strValue;
}
}; // namespace paxoslib