#pragma once
#include "paxoslib/context.hpp"
#include "paxoslib/role.hpp"
namespace paxoslib::role
{
class Learner : public paxoslib::role::Role
{
public:
  Learner(InstanceImpl *pInstance, Context &oContext);
  int OnLearn(const Message &oMessage);
  bool IsLearned() const;

protected:
  virtual int OnReceiveMessage(const Message &oMessage);
  virtual void InitForNewInstance();

private:
  bool m_bLearned;
  Context &m_oContext;
};
}; // namespace paxoslib::role