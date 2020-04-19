#include <spdlog/spdlog.h>
#include "paxoslib/role/learner.hpp"
#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"

namespace paxoslib::role
{
Learner::Learner(InstanceImpl *pInstance) : Role(pInstance)
{
}
int Learner::OnReceiveMessage(const Message &oMessage)
{
  switch (oMessage.type())
  {
  case Message_Type::Message_Type_LEARN:
    this->OnLearn(oMessage);
    break;
  }
}
int Learner::OnLearn(const Message &oMessage)
{
  SPDLOG_DEBUG("Learner OnLearn {}", oMessage.ShortDebugString());
  //重入怎么办？
  if (oMessage.from_node_id() == this->m_pInstance->GetNodeId())
  {
    m_pInstance->SubmitResult();
  }
}
}; // namespace paxoslib::role