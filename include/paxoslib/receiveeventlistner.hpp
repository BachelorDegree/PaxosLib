#pragma once
namespace paxoslib
{
class Message;
};
namespace paxoslib::network
{
class ReceiveEventListener
{
public:
  virtual int OnMessage(const Message &oMessage) = 0;
  virtual ~ReceiveEventListener(){};
};
}; // namespace paxoslib::network