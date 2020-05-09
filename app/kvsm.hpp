#pragma once
#include "paxoslib/statemachine.hpp"
#include "kvclient.hpp"
#include "kvsm.pb.h"
class KVSM : public paxoslib::StateMachine
{
public:
  KVSM(KVClient *pClient)
  {
    m_pClient = pClient;
  }
  ~KVSM()
  {
  }
  virtual int GetID()
  {
    return 1;
  }
  virtual int Execute(uint32_t iGroupIndex, uint64_t instance_id, const std::string &strChosenValue)
  {
    kvsm::Request oRequest;
    bool bResult = oRequest.ParseFromString(strChosenValue);
    if (!bResult)
    {
      SPDLOG_ERROR("Parse from string failed.");
      return 0;
    }
    kvsm::Response oResponse;
    int iRet;
    switch (oRequest.op())
    {
    case kvsm::Request_OpType_Put:
      iRet = DoPut(iGroupIndex, oRequest, oResponse);
      break;
    case kvsm::Request_OpType_Del:
      iRet = DoDel(iGroupIndex, oRequest, oResponse);
      break;
    default:
      SPDLOG_ERROR("Unknown op {}", oRequest.ShortDebugString());
    }
    if (iRet != 0)
    {
      SPDLOG_ERROR("Execute Failed {} {} {}", iRet, oRequest.ShortDebugString(), oResponse.ShortDebugString());
      return iRet;
    }
    SPDLOG_DEBUG("Execute done. {} {}", oRequest.ShortDebugString(), oResponse.ShortDebugString());
    uint64_t a = instance_id;
    std::string s_a((char *)&a, sizeof(a));
    this->m_pClient->Set("CPID" + std::to_string(iGroupIndex), s_a);
    return 0;
  }
  int DoPut(uint32_t iGroupIndex, const kvsm::Request &oReq, kvsm::Response &oResp)
  {
    int iRet = m_pClient->Set(oReq.key(), oReq.value());
    if (iRet != 0)
    {
      SPDLOG_ERROR("kv put failed. oReq:{}", oReq.ShortDebugString());
      return iRet;
    }
    return 0;
  }
  int DoDel(uint32_t iGroupIndex, const kvsm::Request &oReq, kvsm::Response &oResp)
  {
    int iRet = m_pClient->Del(oReq.key());
    if (iRet != 0)
    {
      SPDLOG_ERROR("kv del failed. oReq:{}", oReq.ShortDebugString());
      return iRet;
    }
    return 0;
  }
  virtual int ExecuteForCheckpointReplay(uint32_t iGroupIndex, uint64_t instance_id, const std::string &strChosenValue)
  {
    return 0;
  }
  virtual uint64_t GetCheckpointInstanceID(uint32_t iGroupIndex)
  {
    std::string value;
    int iRet = this->m_pClient->Get("CPID" + std::to_string(iGroupIndex), value);
    assert(iRet == 0 || iRet == 404);
    if (iRet == 404)
    {
      return 0;
    }
    assert(value.size() == sizeof(uint64_t));
    uint64_t a;
    memcpy(&a, value.data(), value.size());
    return a;
  }
  virtual int LockCheckpointState(uint32_t iGroupIndex)
  {
    return 0;
  }
  virtual void UnlockCheckpointState(uint32_t iGroupIndex)
  {
  }
  virtual int LoadCheckpointState(uint32_t iGroupIndex, const std::string &strPath, const std::vector<std::string> &vecFiles)
  {
    return 0;
  }
  virtual int GetCheckpointState(uint32_t iGroupIndex, std::string &strPath, std::vector<std::string> &vecFiles)
  {
    return 0;
  }

private:
  KVClient *m_pClient = nullptr;
};