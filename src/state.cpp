#include <spdlog/spdlog.h>
#include "paxoslib/persistence/state.hpp"
#include "paxoslib/proto/instance.pb.h"
namespace paxoslib
{
namespace persistence
{

State::State(Storage *pStorage) : m_pStorage(pStorage)
{
  Reset();
}
void State::Reset()
{
  m_bPromised = false;
  m_bAccepted = false;
  m_ddwPromisedProposalId = 0;
  m_oAcceptedProposal.Clear();
}
void State::SetState(const paxoslib::persistence::StateProto &oState)
{
  m_bPromised = oState.promised();
  m_bAccepted = oState.accepted();
  m_ddwPromisedProposalId = oState.promised_proposal_id();
  m_oAcceptedProposal.CopyFrom(oState.accepted_proposal());
}
bool State::IsPromised() const { return m_bPromised; }
bool State::IsAccepted() const { return m_bAccepted; }
uint64_t State::GetPromisedProposalId() const { return m_ddwPromisedProposalId; }
const Proposal &State::GetAcceptedProposal() const { return m_oAcceptedProposal; }
void State::SetPromised(bool value) { m_bPromised = value; }
void State::SetAccepted(bool value) { m_bAccepted = value; }
void State::SetPromisedProposalId(uint64_t value) { m_ddwPromisedProposalId = value; }
void State::SetAcceptedProposal(const Proposal &value) { m_oAcceptedProposal = value; }
int State::Persist(uint64_t id)
{
  StateProto oState;
  oState.set_id(id);
  oState.set_promised(m_bPromised);
  oState.set_accepted(m_bAccepted);
  oState.set_promised_proposal_id(m_ddwPromisedProposalId);
  oState.mutable_accepted_proposal()->CopyFrom(m_oAcceptedProposal);
  int iRet = m_pStorage->SaveState(id, oState);
  return iRet;
}
int State::LoadState(uint64_t instance_id)
{
  StateProto oState;
  int iRet = m_pStorage->LoadState(instance_id, oState);
  if (iRet != 0)
  {

    SPDLOG_ERROR("load instance {} failed. ret: {}", instance_id, iRet);
    return iRet;
  }
  this->SetState(oState);
  return 0;
}
}; // namespace persistence
}; // namespace paxoslib