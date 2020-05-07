#include <spdlog/spdlog.h>
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/instanceimpl.hpp"
namespace paxoslib::role
{
Accepter::Accepter(InstanceImpl *pInstance) : Role(pInstance), m_oState(pInstance->m_pStorage.get())
{
}
void Accepter::ReplyRejectPromise(const Message &oMessage)
{
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_REJECT_PREOMISE);
  oNewMessage.set_instance_id(GetInstanceID());
  oNewMessage.mutable_reject_promise()->set_promised_proposal_id(m_oState.GetPromisedProposalId());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
void Accepter::ReplyPromised(const Message &oMessage)
{

  Message oNewMessage;
  oNewMessage.set_type(Message_Type_PROMISED);
  oNewMessage.set_instance_id(GetInstanceID());
  if (m_oState.IsAccepted())
  {
    oNewMessage.mutable_promised()->set_is_accepted(true);
    *oNewMessage.mutable_promised()->mutable_accepted_proposal() = m_oState.GetAcceptedProposal();
  }
  else
  {
    oNewMessage.mutable_promised()->set_is_accepted(false);
  }
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
void Accepter::ReplyRejectAccept(const Message &oMessage)
{
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_REJECT_ACCEPT);
  oNewMessage.set_instance_id(GetInstanceID());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
void Accepter::ReplyAccepted(const Message &oMessage)
{

  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ACCEPTED);
  oNewMessage.set_instance_id(GetInstanceID());
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
int Accepter::OnPrepare(const Message &oMessage)
{
  if (m_oState.IsPromised() == false || m_oState.GetPromisedProposalId() <= oMessage.prepare().proposal_id())
  {
    m_oState.SetPromised(true);
    m_oState.SetPromisedProposalId(oMessage.prepare().proposal_id());
    m_oState.Persist(this->GetInstanceID());
    this->ReplyPromised(oMessage);
  }
  else
  {
    this->ReplyRejectPromise(oMessage);
  }
  return 0;
}
int Accepter::OnAccept(const Message &oMessage)
{
  if (m_oState.GetPromisedProposalId() <= oMessage.accept().proposal().id())
  {
    m_oState.SetAccepted(true);
    m_oState.SetAcceptedProposal(oMessage.accept().proposal());
    m_oState.Persist(this->GetInstanceID());
    this->ReplyAccepted(oMessage);
  }
  else
  {
    this->ReplyRejectAccept(oMessage);
  }
}
int Accepter::OnReceiveMessage(const Message &oMessage)
{
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_ACCEPT:
    this->OnAccept(oMessage);
    break;
  case Message_Type::Message_Type_PREAPRE:
    this->OnPrepare(oMessage);
    break;
  }
}
void Accepter::SetState(const paxoslib::StateProto &oState)
{
  m_oState.SetState(oState);
}
const paxoslib::persistence::State &Accepter::GetState() const
{
  return m_oState;
}
void Accepter::InitForNewInstance()
{
  m_oState.Reset();
}
}; // namespace paxoslib::role
