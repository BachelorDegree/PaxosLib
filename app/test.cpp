#include <unistd.h>
#include <iostream>
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/instance.hpp"
#include "paxoslib/network.hpp"
#include "paxoslib/persistence/storageleveldb.hpp"
#include "paxoslib/statemachinemgr.hpp"
#include "kvsm.hpp"
#include "kvclient.hpp"
void MakeInstance(paxoslib::config::Config oConfig)
{
}
int main()
{
  paxoslib::config::Config oConfig;
  {
    auto pPeer = oConfig.add_peers();
    pPeer->set_id(1);
    pPeer->set_ip("127.0.0.1");
    pPeer->set_port(10001);
  }
  {
    auto pPeer = oConfig.add_peers();
    pPeer->set_id(2);
    pPeer->set_ip("127.0.0.1");
    pPeer->set_port(10002);
  }
  {
    auto pPeer = oConfig.add_peers();
    pPeer->set_id(3);
    pPeer->set_ip("127.0.0.1");
    pPeer->set_port(10003);
  }
  auto oConfig1 = oConfig;
  auto oConfig2 = oConfig;
  auto oConfig3 = oConfig;
  oConfig1.set_node_id(1);
  oConfig1.set_port(10001);
  auto network1 = std::make_shared<paxoslib::network::Network>(oConfig1);
  oConfig2.set_node_id(2);
  oConfig2.set_port(10002);
  auto network2 = std::make_shared<paxoslib::network::Network>(oConfig2);
  oConfig3.set_node_id(3);
  oConfig3.set_port(10003);
  auto network3 = std::make_shared<paxoslib::network::Network>(oConfig3);
  //sleep(1);
  KVClient oClient1("db1");
  KVClient oClient2("db2");
  KVClient oClient3("db3");
  KVSM oSM1(&oClient1);
  KVSM oSM2(&oClient2);
  KVSM oSM3(&oClient3);
  auto pMgr1 = std::make_shared<paxoslib::StateMachineMgr>();
  auto pMgr2 = std::make_shared<paxoslib::StateMachineMgr>();
  auto pMgr3 = std::make_shared<paxoslib::StateMachineMgr>();
  pMgr1->AddStateMachine(&oSM1);
  pMgr2->AddStateMachine(&oSM2);
  pMgr3->AddStateMachine(&oSM3);
  auto pStorage1 = std::unique_ptr<paxoslib::persistence::StorageLeveldb>(new paxoslib::persistence::StorageLeveldb{"/tmp/db1"});
  auto pStorage2 = std::unique_ptr<paxoslib::persistence::StorageLeveldb>(new paxoslib::persistence::StorageLeveldb{"/tmp/db2"});
  auto pStorage3 = std::unique_ptr<paxoslib::persistence::StorageLeveldb>(new paxoslib::persistence::StorageLeveldb{"/tmp/db3"});
  paxoslib::Instance oInstance1{oConfig1, network1, std::move(pStorage1), pMgr1};
  paxoslib::Instance oInstance2{oConfig2, network2, std::move(pStorage2), pMgr2};
  paxoslib::Instance oInstance3{oConfig3, network3, std::move(pStorage3), pMgr3};
  //sleep(1);
  auto iTime = time(nullptr);
  int cnt = 0;
  int i = 0;
  while (true)
  {

    kvsm::Request oReq;
    oReq.set_op(kvsm::Request_OpType_Put);
    oReq.set_key(std::to_string(i++));
    oReq.set_value(std::to_string(i++));
    std::string strReq;
    oReq.SerializeToString(&strReq);
    auto iRet = oInstance1.Propose(strReq);
    SPDLOG_INFO("Propose {} result: {}", i, iRet);
    if (i > 50)
    {
      SPDLOG_INFO("done 50");
      sleep(10);
      return 0;
    }
    //return 0;
  }
}