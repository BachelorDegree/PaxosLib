#pragma once
#include "paxoslib/role.hpp"
#include "paxoslib/util/ballot.hpp"
namespace paxoslib::role
{
class Proposer : public paxoslib::role::Role
{
public:
  Proposer(Instance *pInstance);
  int Propose(const Proposal &oProposal);
  int Accept(const Proposal &oProposal);
  int OnPromised(const Message &oMessage);
  int OnRejectPromise(const Message &oMessage);
  int OnAccepted(const Message &oMessage);
  int OnRejectAccept(const Message &oMessage);
  virtual void OnMessage(const Message &oMessage);
  uint64_t NewProposalId();

private:
  uint64_t GetProposalId();
  uint64_t m_ddwNodeId;
  Proposal m_oProposal;
  util::Ballot m_oBallot;
};
}; // namespace paxoslib::role