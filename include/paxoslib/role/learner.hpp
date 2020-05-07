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
  int OnAskForLearn(const Message &oMessage);
  int OnAskForLearnReply(const Message &oMessage);
  int OnAskForInstanceID(const Message &oMessage);
  int OnAskForInstanceIDReply(const Message &oMessage);

  int AskForLearn(uint16_t node_id);
  int AskForInstanceID();
  bool IsLearned() const;

protected:
  void RejectAskForLearn(const Message &oMessage);
  virtual int OnReceiveMessage(const Message &oMessage);
  virtual void InitForNewInstance();
  virtual void OnSeenOthersInstanceID(uint16_t node_id, uint64_t instance_id);

private:
  bool m_bLearned;
  Context &m_oContext;
  bool m_bIsAskingForLearn;
  uint64_t m_ddwSeenLargestInstanceId;
  uint16_t m_ddwSeenLatestNodeId;
};
}; // namespace paxoslib::role