#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <sys/eventfd.h>
#include "paxoslib/context.hpp"
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/proto/common.pb.h"
#include "paxoslib/persistence/storage.hpp"
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/role/learner.hpp"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "paxoslib/statemachine.hpp"
#include "paxoslib/statemachinemgr.hpp"
#include "paxoslib/eventloop/eventreceiver.hpp"
#include "paxoslib/eventloop/eventloop.hpp"
namespace paxoslib
{
class NodeImpl;
class InstanceImpl : public eventloop::EventReceiver
{

public:
  InstanceImpl(uint32_t iGroupIndex, NodeImpl *pNode, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork, std::unique_ptr<paxoslib::persistence::Storage> pStorage, std::shared_ptr<StateMachineMgr> pStateMachineMgr);
  int Init();
  virtual int OnMessage(const Message &oMessage);
  virtual int OnProposerMessage(const Message &oMessage);
  virtual int OnAccepterMessage(const Message &oMessage);
  virtual int OnLearnerMessage(const Message &oMessage);
  virtual void OnEvent(int iEventType, void *data);
  virtual void OnTimeout(int iEventType, void *data);
  void EnqueuePackageEventloop(char *pBuffer, uint32_t size);
  uint64_t AddTimeout(uint64_t msTime, int iEventType, void *data);
  void RemoveTimeout(uint64_t id);
  void NewInstance();
  uint16_t GetNodeId() const;
  uint32_t GetGroupIndex() const;
  int Propose(const std::string &value, uint64_t &ddwInstanceId);

  std::vector<std::shared_ptr<network::Peer>> GetPeers() const;

private:
  int ExecuteStateMachine(uint64_t id, const std::string &value);
  int OnLearnNewValue(uint64_t id, const std::string &value);
  uint16_t m_ddwNodeId;
  uint64_t m_propose_timeout_id;
  uint32_t m_iGroupIndex;
  NodeImpl *m_pNode;
  std::shared_ptr<network::Network> m_pNetwork;
  std::unique_ptr<paxoslib::persistence::Storage> m_pStorage;
  std::shared_ptr<StateMachineMgr> m_pStateMachineMgr;
  role::Proposer m_oProposer;
  role::Accepter m_oAccepter;
  role::Learner m_oLearner;
  eventloop::EventLoop m_oEventLoop;
  Context m_oContext;
  friend class role::Learner;
  friend class role::Proposer;
  friend class role::Accepter;
  friend class role::Role;
};
}; // namespace paxoslib