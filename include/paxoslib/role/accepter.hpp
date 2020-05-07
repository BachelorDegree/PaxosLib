#pragma once
#include "paxoslib/role.hpp"
#include "paxoslib/persistence/state.hpp"
namespace paxoslib::role
{
class Accepter : public paxoslib::role::Role
{
public:
  Accepter(InstanceImpl *pInstance);
  int OnPrepare(const Message &oMessage);
  int OnAccept(const Message &oMessage);
  void SetState(const paxoslib::StateProto &oState);
  const paxoslib::persistence::State &GetState() const;

protected:
  virtual int OnReceiveMessage(const Message &oMessage);
  virtual void InitForNewInstance();

private:
  paxoslib::persistence::State m_oState;
  void ReplyPromised(const Message &oMessage);
  void ReplyRejectPromise(const Message &oMessage);
  void ReplyAccepted(const Message &oMessage);
  void ReplyRejectAccept(const Message &oMessage);
};
}; // namespace paxoslib::role