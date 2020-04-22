#pragma once
#include "paxoslib/role.hpp"
#include "paxoslib/util/ballot.hpp"
namespace paxoslib::role
{
class Proposer : public paxoslib::role::Role
{
public:
  Proposer(InstanceImpl *pInstance);
  int Propose(const Proposal &oProposal);
  int Accept(const Proposal &oProposal);
  int Learn(const Proposal &oProposal);
  int OnPromised(const Message &oMessage);
  int OnRejectPromise(const Message &oMessage);
  int OnAccepted(const Message &oMessage);
  int OnRejectAccept(const Message &oMessage);
  uint64_t NewProposalId();

protected:
  virtual int OnReceiveMessage(const Message &oMessage);
  uint64_t RespawnProposalId();
  uint64_t GetProposalId();

private:
  uint16_t m_ddwNodeId;
  uint64_t m_ddwProposalId;
  int m_iStage;
  Proposal m_oProposal;
  util::Ballot m_oBallot;
};
}; // namespace paxoslib::role