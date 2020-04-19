#pragma once
#include "paxoslib/role.hpp"
namespace paxoslib::role
{
class Learner : public paxoslib::role::Role
{
public:
  Learner(InstanceImpl *pInstance);
  int OnLearn(const Message &oMessage);

protected:
  virtual int OnReceiveMessage(const Message &oMessage);

private:
};
}; // namespace paxoslib::role