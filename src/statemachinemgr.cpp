#include "paxoslib/statemachinemgr.hpp"
namespace paxoslib
{

StateMachineMgr::StateMachineMgr() {}
StateMachineMgr::~StateMachineMgr() {}
void StateMachineMgr::AddStateMachine(StateMachine *pSM)
{
  m_vecpSM.push_back(pSM);
}
int StateMachineMgr::Execute(int smid, uint64_t instance_id, const std::string &strChosenValue)
{
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetID() == smid || true)
    {
      return this->ExecuteForSM(pSM, instance_id, strChosenValue);
    }
  }
  return -1;
}
int StateMachineMgr::ExecuteForSM(StateMachine *pSM, uint64_t instance_id, const std::string &strChosenValue)
{
  return pSM->Execute(instance_id, strChosenValue);
}
int StateMachineMgr::GetCheckpointMinInstanceID(uint64_t &instance_id) const
{
  instance_id = UINT64_MAX;
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetCheckpointInstanceID() < instance_id)
    {
      instance_id = pSM->GetCheckpointInstanceID();
    }
  }
  return 0;
}
int StateMachineMgr::ExecuteForReplay(int smid, uint64_t instance_id, const std::string &strChosenValue)
{
  for (auto pSM : m_vecpSM)
  {
    if (pSM->GetID() == smid || true)
    {
      if (pSM->GetCheckpointInstanceID() < instance_id)
      {
        return this->ExecuteForSM(pSM, instance_id, strChosenValue);
      }
    }
  }
  return -1;
} // namespace paxoslib
}; // namespace paxoslib