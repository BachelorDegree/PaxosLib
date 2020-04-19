#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include "paxoslib/proto/message.pb.h"
#include "paxoslib/instance.hpp"
#include "paxoslib/role/accepter.hpp"
#include "paxoslib/role/learner.hpp"
#include "paxoslib/role/proposer.hpp"
#include "paxoslib/peer.hpp"
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/network.hpp"
#include "paxoslib/channel.hpp"
#include "sys/eventfd.h"

namespace paxoslib
{

class InstanceImpl : public network::ReceiveEventListener
{

public:
  InstanceImpl(Instance *pInstance, const paxoslib::config::Config &oConfig, std::shared_ptr<network::Network> pNetwork);
  virtual int OnMessage(const Message &oMessage);
  void NewInstance();
  uint64_t GetInstanceId() const;
  const Proposal &GetProposal() const;
  uint64_t GetNodeId() const;
  int Propose(const std::string &value);
  std::vector<std::shared_ptr<network::Peer>> GetPeers() const;

private:
  int WaitResult();
  int SubmitResult();
  uint64_t m_ddwInstanceId;
  uint64_t m_ddwNodeId;
  int m_event_fd;
  Proposal m_oProposal;
  std::vector<std::shared_ptr<network::Peer>> m_vecPeers;
  std::shared_ptr<network::Network> m_pNetwork;
  role::Proposer m_oProposer;
  role::Accepter m_oAccepter;
  role::Learner m_oLearner;
  friend class role::Learner;
};
}; // namespace paxoslib