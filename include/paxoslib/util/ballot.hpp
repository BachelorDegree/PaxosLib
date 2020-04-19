#pragma once
#include <set>
#include <cstdint>
namespace paxoslib
{
namespace util
{
class Ballot
{
public:
  Ballot()
  {
  }
  void ResetBallot(uint32_t total)
  {
    m_ddwTotal = total;
    m_setUpVoter.clear();
    m_setDownVoter.clear();
    m_ddwMaxProposalId = 0;
    m_oChosenProposal.Clear();
    m_bHasOtherProposal = false;
  }
  void VoteUp(uint32_t voter)
  {
    m_setUpVoter.insert(voter);
  }
  void VoteUp(uint32_t voter, const Proposal &oProposal)
  {
    m_setUpVoter.insert(voter);
    if (oProposal.id() > m_oChosenProposal.id())
    {
      m_oChosenProposal = oProposal;
      m_bHasOtherProposal = true;
    }
  }
  void VoteDown(uint32_t voter, uint64_t proposal_id)
  {
    m_setDownVoter.insert(voter);
    if (m_ddwMaxProposalId < proposal_id)
    {
      m_ddwMaxProposalId = proposal_id;
    }
  }
  const Proposal &GetChosenProposal(const Proposal &oWillingProposal) const
  {
    if (m_bHasOtherProposal)
    {
      return m_oChosenProposal;
    }
    return oWillingProposal;
  }
  uint32_t GetMajority() const
  {
    return m_ddwTotal / 2 + 1;
  }
  bool IsMajorityUp() const
  {
    return m_setUpVoter.size() >= GetMajority();
  }
  bool IsMajorityDown() const
  {
    return m_setDownVoter.size() >= GetMajority();
  }
  uint32_t GetUpCount() const
  {
    return m_setUpVoter.size();
  }
  uint32_t GetDownCount() const
  {
    return m_setDownVoter.size();
  }

private:
  uint32_t m_ddwTotal;
  std::set<uint32_t> m_setUpVoter;
  std::set<uint32_t> m_setDownVoter;
  uint32_t m_ddwMaxProposalId;
  Proposal m_oChosenProposal;
  bool m_bHasOtherProposal;
};
}; // namespace util

}; // namespace paxoslib