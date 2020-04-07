#include "paxoslib/proto/message.pb.h"
#include "paxoslib/role/proposer.hpp"
using namespace paxoslib;
namespace paxoslib::role
{
Proposer::Proposer(Instance *pInstance) : Role(pInstance) {}
uint64_t Proposer::GetProposalId()
{
  return 1;
}
int Proposer::Propose(const Proposal &oProposal)
{
  Message oMessage;
  this->m_oProposal = oProposal;
  oMessage.set_type(Message_Type::Message_Type_PREAPRE);
  auto &oPrepare = *oMessage.mutable_prepare();
  oPrepare.set_proposal_id(this->GetProposalId());
  this->m_oBallot.ResetBallot(3, oProposal);
  this->Broadcast(BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_ACCEPTER, BroadcastType::BROAD_CAST_TYPE_ALL, oMessage);
}
int Proposer::Accept(const Proposal &oProposal)
{
  Message oMessage;
  oMessage.set_type(Message_Type::Message_Type_ACCEPT);
  oMessage.mutable_accept()->mutable_proposal()->CopyFrom(oProposal);
  oMessage.mutable_accept()->mutable_proposal()->set_id(this->GetProposalId());
  this->Broadcast(BroadcastReceiverType::BORADCAST_RECEIVER_TYPE_ACCEPTER, BroadcastType::BROAD_CAST_TYPE_ALL, oMessage);
}
int Proposer::OnPromised(const Message &oMessage)
{
  if (oMessage.promised().has_accepted_proposal())
  {
    m_oBallot.VoteUp(oMessage.from_node_id(), oMessage.promised().accepted_proposal());
  }
  else
  {
    m_oBallot.VoteUp(oMessage.from_node_id());
  }
  if (m_oBallot.IsMajorityUp())
  {
    this->Accept(m_oBallot.GetChosenProposal(this->m_oProposal));
  }
}
int Proposer::OnRejectPromise(const Message &oMessage)
{
  m_oBallot.VoteDown(oMessage.from_node_id(), oMessage.reject_promise().promised_proposal_id());
  if (m_oBallot.IsMajorityDown())
  {
  }
}
int Proposer::OnAccepted(const Message &oMessage) {}
int Proposer::OnRejectAccept(const Message &oMessage) {}
void Proposer::OnMessage(const Message &oMessage)
{
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_PROMISED:
    this->OnPromised(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_PREOMISE:
    this->OnRejectPromise(oMessage);
    break;
  case Message_Type::Message_Type_ACCEPTED:
    this->OnAccepted(oMessage);
    break;
  case Message_Type::Message_Type_REJECT_ACCEPT:
    this->OnRejectAccept(oMessage);
    break;
  }
}
}; // namespace paxoslib::role