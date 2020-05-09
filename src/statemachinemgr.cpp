#include "paxoslib/statemachinemgr.hpp"
namespace paxoslib
{

StateMachineMgr::StateMachineMgr() {}
StateMachineMgr::~StateMachineMgr() {}
void StateMachineMgr::AddStateMachine(StateMachine *pSM)
{
  m_vecpSM.push_back(pSM);
}
int StateMachineMgr::Execute(uint32_t iGroupIndex, int smid, uint64_t instance_id, const std::string &strChosenValue)
{
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetID() == smid || true)
    {
      return this->ExecuteForSM(iGroupIndex, pSM, instance_id, strChosenValue);
    }
  }
  return -1;
}
int StateMachineMgr::ExecuteForSM(uint32_t iGroupIndex, StateMachine *pSM, uint64_t instance_id, const std::string &strChosenValue)
{
  return pSM->Execute(iGroupIndex, instance_id, strChosenValue);
}
int StateMachineMgr::GetCheckpointMinInstanceID(uint32_t iGroupIndex, uint64_t &instance_id) const
{
  instance_id = UINT64_MAX;
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetCheckpointInstanceID(iGroupIndex) < instance_id)
    {
      instance_id = pSM->GetCheckpointInstanceID(iGroupIndex);
    }
  }
  return 0;
}
int StateMachineMgr::ExecuteForReplay(uint32_t iGroupIndex, int smid, uint64_t instance_id, const std::string &strChosenValue)
{
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetID() == smid || true)
    {
      if (pSM->GetCheckpointInstanceID(iGroupIndex) < instance_id)
      {
        return this->ExecuteForSM(iGroupIndex, pSM, instance_id, strChosenValue);
      }
    }
  }
  return -1;
} // namespace paxoslib
}; // namespace paxoslib