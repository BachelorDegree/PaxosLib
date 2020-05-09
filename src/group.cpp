#include <cstring>
#include <netinet/in.h>
#include "paxoslib/group.hpp"
#include "paxoslib/instanceimpl.hpp"

namespace paxoslib
{
Group::Group(uint32_t iGroupSize)
{
  m_iGroupSize = iGroupSize;
  this->m_vecInstances.reserve(m_iGroupSize);
  for (int i = 0; i < iGroupSize; i++)
  {
    this->m_vecInstances[i] = nullptr;
  }
}
Group::~Group()
{
  for (auto pInstance : m_vecInstances)
  {
    if (pInstance)
    {
      delete pInstance;
    }
  }
}
uint32_t Group::GetGroupSize() const
{
  return m_iGroupSize;
}
InstanceImpl *Group::GetInstance(uint32_t iGroupIndex)
{
  if (iGroupIndex >= m_iGroupSize)
  {
    return nullptr;
  }
  return this->m_vecInstances[iGroupIndex];
}
InstanceImpl *Group::MakeInstance(uint32_t iGroupIndex, NodeImpl *pNode, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork, std::unique_ptr<paxoslib::persistence::Storage> pStorage, std::shared_ptr<StateMachineMgr> pStateMachineMgr)
{
  auto pInstance = new InstanceImpl(iGroupIndex, pNode, oConfig, pNetwork, std::move(pStorage), pStateMachineMgr);
  int iRet = pInstance->Init();
  if (iRet != 0)
  {
    delete pInstance;
    return nullptr;
  }
  this->m_vecInstances[iGroupIndex] = pInstance;
  return pInstance;
}
int Group::Propose(uint32_t iGroupIndex, const std::string &strValue, uint64_t &ddwInstanceId)
{
  auto pInstance = GetInstance(iGroupIndex);
  if (pInstance == nullptr)
  {
    return 404;
  }
  return pInstance->Propose(strValue, ddwInstanceId);
}
int Group::DispatchPackage(uint32_t iGroupIndex, char *pBuffer, uint32_t size)
{
  auto pInstance = this->GetInstance(iGroupIndex);
  if (pInstance == nullptr)
  {
    return 404;
  }
  pInstance->EnqueuePackageEventloop(pBuffer, size);
  return 0;
}

}; // namespace paxoslib