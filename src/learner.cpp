#include <spdlog/spdlog.h>
#include "paxoslib/role/learner.hpp"
#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/persistence/state.hpp"
#include "paxoslib/eventloop/eventtype.hpp"
namespace paxoslib::role
{
Learner::Learner(InstanceImpl *pInstance, Context &oContext) : Role(pInstance), m_oContext(oContext)
{
  m_bLearned = false;
  m_ddwSeenLargestInstanceId = 0;
  m_ddwSeenLatestNodeId = 0;
  m_bIsAskingForLearn = false;
}
int Learner::OnReceiveMessage(const Message &oMessage)
{
  int iRet;
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_LEARN:
    iRet = this->OnLearn(oMessage);
    break;
  case Message_Type::Message_Type_ASK_FOR_INSTANCE_ID:
    iRet = this->OnAskForInstanceID(oMessage);
    break;
  case Message_Type::Message_Type_ASK_FOR_INSTANCE_ID_REPLY:
    iRet = this->OnAskForInstanceIDReply(oMessage);
    break;
  case Message_Type::Message_Type_ASK_FOR_LEARN_REPLY:
    iRet = this->OnAskForLearnReply(oMessage);
    break;
  case Message_Type::Message_Type_ASK_FOR_LEARN:
    iRet = this->OnAskForLearn(oMessage);
    break;
  }
  return iRet;
}
int Learner::OnLearn(const Message &oMessage)
{
  SPDLOG_DEBUG("Learner OnLearn {}", oMessage.ShortDebugString());
  //重入怎么办？
  if (m_bLearned)
  {
    SPDLOG_INFO("Learned, skip");
    return 0;
  }
  m_bLearned = true;
  return 0;
}
bool Learner::IsLearned() const
{
  return m_bLearned;
}
void Learner::InitForNewInstance()
{
  m_bLearned = false;
}
int Learner::OnAskForLearn(const Message &oMessage)
{
  if (oMessage.ask_for_learn().from_instance_id() >= GetInstanceID())
  {
    this->RejectAskForLearn(oMessage);
    return 0;
  }
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ASK_FOR_LEARN_REPLY);
  oNewMessage.mutable_ask_for_learn_reply()->set_start_instance_id(oMessage.ask_for_learn().from_instance_id());
  auto pStates = oNewMessage.mutable_ask_for_learn_reply()->mutable_states();
  auto pStorage = m_pInstance->m_pStorage.get();
  paxoslib::persistence::State oState(pStorage);
  int cnt = 0;
  for (uint64_t id = oMessage.ask_for_learn().from_instance_id(); id < GetInstanceID(); id++)
  {
    int iRet = oState.LoadState(id);
    if (iRet != 0)
    {
      SPDLOG_ERROR("Load state failed id:{} ret:{}", id, iRet);
      return iRet;
    }
    if (id != oMessage.ask_for_learn().from_instance_id() && oState.GetAcceptedProposal().ByteSize() + oNewMessage.ByteSize() > 1024 * 1024)
    {
      SPDLOG_DEBUG("Size limit exceed. skip");
      break;
    }
    auto oStateProto = oState.GetState();
    oStateProto.set_id(id);
    pStates->Add()->CopyFrom(oStateProto);
  }
  SPDLOG_DEBUG("Reply ask for learn. from:{} length:{}", oMessage.ask_for_learn().from_instance_id(), pStates->size());
  oNewMessage.mutable_ask_for_learn_reply()->set_my_instance_id(GetInstanceID());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
  return 0;
}
void Learner::RejectAskForLearn(const Message &oMessage)
{
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ASK_FOR_LEARN_REPLY);
  oNewMessage.set_instance_id(GetInstanceID());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
int Learner::OnAskForLearnReply(const Message &oMessage)
{
  for (const auto &oStateProto : oMessage.ask_for_learn_reply().states())
  {
    if (oStateProto.id() < GetInstanceID())
    {
      continue;
    }
    auto pStorage = m_pInstance->m_pStorage.get();
    paxoslib::persistence::State oState(pStorage);
    oState.SetState(oStateProto);
    int iRet = oState.Persist(oStateProto.id());
    if (iRet != 0)
    {
      SPDLOG_ERROR("Persist state failed id:{} ret:{}", oStateProto.id(), iRet);
      return iRet;
    }
    iRet = this->m_pInstance->OnLearnNewValue(oStateProto.id(), oStateProto.accepted_proposal().value());
    if (iRet != 0)
    {
      SPDLOG_ERROR("OnLearnNewValue failed id:{} ret:{}", oStateProto.id(), iRet);
      return iRet;
    }
    SPDLOG_DEBUG("Learn value id: {}", oStateProto.id());
    this->m_pInstance->NewInstance();
  }
  SPDLOG_ERROR("Learn to {} {} {} {}", GetInstanceID(), oMessage.id(), oMessage.from_node_id(), time(nullptr));
  m_bIsAskingForLearn = false;
  this->OnSeenOthersInstanceID(oMessage.from_node_id(), oMessage.ask_for_learn_reply().my_instance_id());
  return 0;
}
int Learner::OnAskForInstanceID(const Message &oMessage)
{
  OnSeenOthersInstanceID(oMessage.from_node_id(), oMessage.ask_for_instance_id().my_instance_id());
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ASK_FOR_INSTANCE_ID_REPLY);
  oNewMessage.set_instance_id(GetInstanceID());
  oNewMessage.mutable_ask_for_instance_id_reply()->set_my_instance_id(GetInstanceID());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
  return 0;
}

int Learner::OnAskForInstanceIDReply(const Message &oMessage)
{
  OnSeenOthersInstanceID(oMessage.from_node_id(), oMessage.ask_for_instance_id_reply().my_instance_id());
  return 0;
}
int Learner::AskForLearn(uint16_t node_id)
{
  if (m_bIsAskingForLearn)
  {
    return 0;
  }
  m_bIsAskingForLearn = true;
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ASK_FOR_LEARN);
  oNewMessage.set_instance_id(GetInstanceID());
  oNewMessage.mutable_ask_for_learn()->set_from_instance_id(GetInstanceID());
  this->SendMessageTo(node_id, oNewMessage);
  return 0;
}
int Learner::AskForInstanceID()
{
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ASK_FOR_INSTANCE_ID);
  oNewMessage.set_instance_id(GetInstanceID());
  oNewMessage.mutable_ask_for_instance_id()->set_my_instance_id(GetInstanceID());
  this->Broadcast(BORADCAST_RECEIVER_TYPE_LEARNER, BROAD_CAST_TYPE_ALL, oNewMessage);
  m_pInstance->AddTimeout(10000, eventloop::EventType::LearnerAskForInstanceIdLoop, 0);
  return 0;
}
void Learner::OnSeenOthersInstanceID(uint16_t node_id, uint64_t instance_id)
{
  if (this->m_ddwSeenLargestInstanceId < instance_id)
  {
    this->m_ddwSeenLargestInstanceId = instance_id;
    this->m_ddwSeenLatestNodeId = node_id;
  }

  if (GetInstanceID() < this->m_ddwSeenLargestInstanceId)
  {
    this->AskForLearn(this->m_ddwSeenLatestNodeId);
  }
}
}; // namespace paxoslib::role