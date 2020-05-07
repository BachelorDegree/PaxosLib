#pragma once
#include <cstdint>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/common.pb.h"
#include "paxoslib/persistence/storage.hpp"
namespace paxoslib
{
namespace persistence
{
class State
{
public:
  State(Storage *);
  void Reset();
  void SetState(const paxoslib::StateProto &oState);
  paxoslib::StateProto GetState() const;
  int LoadState(uint64_t);
  bool IsPromised() const;
  bool IsAccepted() const;
  uint64_t GetPromisedProposalId() const;
  const Proposal &GetAcceptedProposal() const;
  void SetPromised(bool);
  void SetAccepted(bool);
  void SetPromisedProposalId(uint64_t);
  void SetAcceptedProposal(const Proposal &);
  int Persist(uint64_t id);

private:
  Storage *m_pStorage;
  bool m_bPromised;
  bool m_bAccepted;
  uint64_t m_ddwPromisedProposalId;
  Proposal m_oAcceptedProposal;
};
}; // namespace persistence
}; // namespace paxoslib