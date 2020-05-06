#pragma once
#include "paxoslib/context.hpp"
#include "paxoslib/role.hpp"
#include "paxoslib/util/ballot.hpp"
namespace paxoslib::role
{
class Proposer : public paxoslib::role::Role
{
public:
  Proposer(InstanceImpl *pInstance, Context &oContext);
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
  int OnPrepareFailed();
  int OnAcceptFailed();
  uint64_t RespawnProposalId();
  uint64_t GetProposalId();
  virtual void InitForNewInstance();

private:
  enum class Stage
  {
    Idle = 0,
    Preparing = 1,
    Accepting = 2,

  };
  uint16_t m_ddwNodeId;
  uint64_t m_ddwProposalId;
  Stage m_oStage;
  Proposal m_oProposal;
  util::Ballot m_oBallot;
  Context &m_oContext;
};
}; // namespace paxoslib::role