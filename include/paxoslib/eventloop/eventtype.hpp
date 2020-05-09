#pragma once
namespace paxoslib
{
namespace eventloop
{
enum EventType
{
  InstanceMessage = 1,
  LearnerAskForInstanceIdLoop = 2,
  ProposeTimeout = 3
};
}; // namespace eventloop
}; // namespace paxoslib