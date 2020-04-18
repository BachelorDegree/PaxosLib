#pragma once
#include "paxoslib/role.hpp"
namespace paxoslib::role
{
class Accepter : public paxoslib::role::Role
{
public:
  Accepter(Instance *pInstance);
  int OnPrepare(const Message &oMessage);
  int OnAccept(const Message &oMessage);

protected:
  virtual int OnReceiveMessage(const Message &oMessage);

private:
  void ReplyPromised(const Message &oMessage);
  void ReplyRejectPromise(const Message &oMessage);
  void ReplyAccepted(const Message &oMessage);
  void ReplyRejectAccept(const Message &oMessage);
  bool m_bPromised;
  bool m_bAccepted;
  uint64_t m_ddwPromisedProposalId;
  Proposal m_oAcceptedProposal;
};
}; // namespace paxoslib::role