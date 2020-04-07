#include "paxoslib/role/accepter.hpp"
namespace paxoslib::role
{
Accepter::Accepter(Instance *pInstance) : Role(pInstance)
{
  m_bPromised = false;
  m_bAccepted = false;
}
void Accepter::ReplyRejectPromise(const Message &oMessage)
{
  Message oNewMessage;
  oNewMessage.set_type(Message_Type_REJECT_PREOMISE);
  oNewMessage.mutable_reject_promise()->set_promised_proposal_id(this->m_ddwPromisedProposalId);
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
void Accepter::ReplyPromised(const Message &oMessage)
{

  Message oNewMessage;
  oNewMessage.set_type(Message_Type_PROMISED);
  if (m_bAccepted)
  {
    oNewMessage.mutable_promised()->set_is_accepted(true);
    *oNewMessage.mutable_promised()->mutable_accepted_proposal() = m_oAcceptedProposal;
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
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
void Accepter::ReplyAccepted(const Message &oMessage)
{

  Message oNewMessage;
  oNewMessage.set_type(Message_Type_ACCEPTED);
  this->SendMessageTo(oMessage.from_node_id(), oNewMessage);
}
int Accepter::OnPrepare(const Message &oMessage)
{
  if (this->m_bPromised == false || this->m_ddwPromisedProposalId <= oMessage.prepare().proposal_id())
  {
    this->m_bPromised = true;
    this->m_ddwPromisedProposalId = oMessage.prepare().proposal_id();
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
  if (this->m_ddwPromisedProposalId <= oMessage.accept().proposal().id())
  {
    this->m_bAccepted = true;
    this->m_oAcceptedProposal = oMessage.accept().proposal();
    this->ReplyAccepted(oMessage);
  }
  else
  {
    this->ReplyRejectAccept(oMessage);
  }
}
void Accepter::OnMessage(const Message &oMessage)
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
}; // namespace paxoslib::role
