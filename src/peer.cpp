#include <thread>
#include "paxoslib/peer.hpp"
#include "paxoslib/network.hpp"
#include "sys/eventfd.h"
#include "paxoslib/util/timetrace.hpp"
#include "paxoslib/eventloop/eventtype.hpp"
namespace paxoslib::network
{
Peer::Peer(uint16_t peer_id, network::PackageReceiver *pPackageReceiver, std::shared_ptr<Network> pNetwork)
{
  m_pPackageReceiver = pPackageReceiver;
  m_peer_id = peer_id;
  m_pNetwork = pNetwork;
}
uint32_t Peer::GetPeerID() const
{
  return m_peer_id;
}
const std::set<RoleType> &Peer::GetRoleTypes() const
{
  return m_setRoleTypes;
}
uint64_t Peer::GetRoleTypesMask() const
{
  uint64_t ret = 0;
  for (auto a : m_setRoleTypes)
  {
    ret = ret | (1 << a);
  }
  return ret;
}
void Peer::SendMessage(const Message &oMessage)
{
  Trace::Mark(oMessage.id(), "Peer::SendMessage");
  uint32_t iGroupIndex = htonl(oMessage.group_index());
  uint32_t size = oMessage.ByteSizeLong() + sizeof(iGroupIndex);
  char *pBuffer = new char[size];
  memcpy(pBuffer, (void *)&iGroupIndex, sizeof(iGroupIndex));
  oMessage.SerializeToArray(pBuffer + sizeof(iGroupIndex), size - sizeof(iGroupIndex));
  this->m_pNetwork->SendMessageToPeer(m_peer_id, std::unique_ptr<char[]>(pBuffer), size);
}
int Peer::EnqueueReceiveMessage(std::unique_ptr<char[]> pBuffer, uint32_t size)
{
  return m_pPackageReceiver->OnPackage(pBuffer.release(), size);
}
void Peer::AddRoleType(RoleType oType)
{
  this->m_setRoleTypes.insert(oType);
}
}; // namespace paxoslib::network