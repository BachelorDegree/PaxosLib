#pragma once
#include <chrono>
#include <spdlog/spdlog.h>
#include <mutex>
#include <map>
#include <string>
class Trace
{
public:
  static uint64_t
  GetSteadyClockMS()
  {
    auto now = std::chrono::steady_clock::now();
    uint64_t now_ms = (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())).count();
    return now_ms;
  }
  static void Mark(uint64_t id, std::string str)
  {
    static std::mutex oMutex;
    static std::map<uint64_t, uint64_t> oMap;
    std::lock_guard<std::mutex> oGuard(oMutex);
    if (oMap.count(id) > 0 && GetSteadyClockMS() - oMap[id] > 2)
    {
      SPDLOG_ERROR("Trace: {} {}ms {}", id, GetSteadyClockMS() - oMap[id], str);
    }
    oMap[id] = GetSteadyClockMS();
  }
};