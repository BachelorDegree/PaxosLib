#pragma once
#include <functional>
namespace paxoslib
{
namespace eventloop
{
class EventLoop;
class EventReceiver
{
public:
  virtual ~EventReceiver(){};
  virtual void OnEvent(int iEventType, void *data) = 0;
  virtual void OnTimeout(int iEventType, void *data) = 0;
  // virtual void AddTimeout(int usTime, int iEventType, void *data);
  // virtual void AddEventToTail(int iEventType, void *data);
};
}; // namespace eventloop
}; // namespace paxoslib