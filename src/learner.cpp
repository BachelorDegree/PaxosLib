#include <spdlog/spdlog.h>
#include "paxoslib/role/learner.hpp"
#include "paxoslib/instanceimpl.hpp"
#include "paxoslib/proto/message.pb.h"

namespace paxoslib::role
{
Learner::Learner(InstanceImpl *pInstance, Context &oContext) : Role(pInstance), m_oContext(oContext)
{
  m_bLearned = false;
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
  if (m_bLearned)
  {
    SPDLOG_INFO("Learned, skip");
    return 0;
  }
  m_bLearned = true;
}
bool Learner::IsLearned() const
{
  return m_bLearned;
}
void Learner::InitForNewInstance()
{
  m_bLearned = false;
}
}; // namespace paxoslib::role