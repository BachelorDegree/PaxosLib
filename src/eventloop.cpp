#include <thread>
#include <unistd.h>
#include <chrono>
#include <spdlog/spdlog.h>
#include <sys/prctl.h>
#include "paxoslib/eventloop/eventloop.hpp"
#include "paxoslib/eventloop/eventreceiver.hpp"
namespace paxoslib
{
namespace eventloop
{
uint64_t GetSteadyClockMS()
{
  auto now = std::chrono::steady_clock::now();
  uint64_t now_ms = (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())).count();
  return now_ms;
}
struct EventLoop::EventItem
{
  EventReceiver *pEventReceiver;
  int iEventType;
  void *data;
};
struct EventLoop::TimeoutItem
{
  EventReceiver *pEventReceiver;
  int iEventType;
  void *data;
  uint64_t msTime;
};
EventLoop::EventLoop()
{
  m_ddwTimeoutIdCnt = 0;
}
EventLoop::~EventLoop() {}
void EventLoop::Start()
{
  auto thread = std::thread([&]() {
    prctl(PR_SET_NAME, "eventloop");
    while (true)
    {
      this->ProcessTimeout();
      this->ProcessEvent();
      usleep(1);
    }
  });
  thread.detach();
}
void EventLoop::Stop() {}
void EventLoop::AddEventTail(EventReceiver *pEventReceiver, int iEventType, void *data)
{
  auto pEventItem = new EventItem;
  pEventItem->data = data;
  pEventItem->iEventType = iEventType;
  pEventItem->pEventReceiver = pEventReceiver;
  m_qEvent.push_back(pEventItem);
}
uint64_t EventLoop::AddTimeout(EventReceiver *pEventReceiver, uint64_t msTime, int iEventType, void *data)
{
  auto pItem = new TimeoutItem;
  pItem->data = data;
  pItem->iEventType = iEventType;
  pItem->pEventReceiver = pEventReceiver;
  pItem->msTime = msTime;
  uint64_t nxt = msTime + GetSteadyClockMS();
  std::lock_guard<std::mutex> oLockGuard{m_TimeoutMutex};
  auto id = m_ddwTimeoutIdCnt++;
  m_mapTimeoutItem[id] = pItem;
  m_heapTimeout.push({nxt, id});
  return id;
}
void EventLoop::RemoveTimeout(uint64_t id)
{
  std::lock_guard<std::mutex> oLockGuard{m_TimeoutMutex};
  if (m_mapTimeoutItem.count(id) > 0)
  {
    m_mapTimeoutItem.erase(id);
  }
}
paxoslib::eventloop::EventLoop::TimeoutItem *EventLoop::PopTimeoutItem()
{
  std::lock_guard<std::mutex> oLockGuard{m_TimeoutMutex};
  while (true)
  {
    if (m_mapTimeoutItem.empty())
    {
      return nullptr;
    }
    auto oPair = m_heapTimeout.top();
    if (oPair.first > GetSteadyClockMS())
    {
      return nullptr;
    }
    m_heapTimeout.pop();
    if (m_mapTimeoutItem.count(oPair.second) == 0)
    {
      continue;
    }
    auto pItem = m_mapTimeoutItem.at(oPair.second);
    m_mapTimeoutItem.erase(oPair.second);
    return pItem;
  }
}
void EventLoop::ProcessTimeout()
{
  TimeoutItem *pItem;
  while ((pItem = this->PopTimeoutItem()) != nullptr)
  {
    pItem->pEventReceiver->OnTimeout(pItem->iEventType, pItem->data);
    delete pItem;
  }
}
void EventLoop::ProcessEvent()
{
  EventItem *pItem;
  //add stub to prevent deadloop
  m_qEvent.push_back(nullptr);
  while (m_qEvent.pop_front(pItem))
  {
    if (pItem == nullptr)
    {
      break;
    }
    pItem->pEventReceiver->OnEvent(pItem->iEventType, pItem->data);
    delete pItem;
  }
}

}; // namespace eventloop
}; // namespace paxoslib