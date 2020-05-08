#pragma once
#include <map>
#include <queue>
#include <mutex>
#include "paxoslib/util/lfqueue.hpp"
namespace paxoslib
{
class InstanceImpl;
namespace eventloop
{
class EventReceiver;
class EventLoop
{
public:
  EventLoop();
  virtual ~EventLoop();
  void Start();
  void Stop();
  void AddEventTail(EventReceiver *pEventReceiver, int iEventType, void *data);
  uint64_t AddTimeout(EventReceiver *pEventReceiver, uint64_t msTime, int iEventType, void *data);
  void RemoveTimeout(uint64_t id);

private:
  struct EventItem;
  struct TimeoutItem;
  uint64_t m_ddwTimeoutIdCnt;
  util::LFQueue<EventItem *> m_qEvent;
  std::map<uint32_t, TimeoutItem *> m_mapTimeoutItem;
  std::priority_queue<std::pair<uint64_t, uint32_t>> m_heapTimeout;
  void ProcessTimeout();
  void ProcessEvent();
  TimeoutItem *PopTimeoutItem();
  std::mutex m_TimeoutMutex;
};
}; // namespace eventloop
}; // namespace paxoslib