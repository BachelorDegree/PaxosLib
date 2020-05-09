#include <unistd.h>
#include <iostream>
#include <atomic>
#include <sys/prctl.h>

#include "paxoslib/proto/config.pb.h"
#include "paxoslib/node.hpp"
#include "paxoslib/statemachinemgr.hpp"
#include "kvsm.hpp"
#include "kvclient.hpp"
std::atomic<uint64_t> g_cnt{0};
int main()
{
  prctl(PR_SET_NAME, "t_main");
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
  oConfig1.set_paxoslog_dir("/tmp/storage/node1/");
  oConfig2.set_node_id(2);
  oConfig2.set_port(10002);
  oConfig2.set_paxoslog_dir("/tmp/storage/node2/");
  oConfig3.set_node_id(3);
  oConfig3.set_port(10003);
  oConfig3.set_paxoslog_dir("/tmp/storage/node3/");
  //sleep(1);
  KVClient oClient1("db1");
  KVClient oClient2("db2");
  KVClient oClient3("db3");
  KVSM oSM1(&oClient1);
  KVSM oSM2(&oClient2);
  KVSM oSM3(&oClient3);
  paxoslib::Node node1(oConfig1);
  paxoslib::Node node2(oConfig2);
  paxoslib::Node node3(oConfig3);

  node1.AddStateMachine(&oSM1);
  node2.AddStateMachine(&oSM2);
  node3.AddStateMachine(&oSM3);
  node1.Init();
  node2.Init();
  node3.Init();
  //sleep(1);
  auto iTime = time(nullptr);
  int cnt = 0;
  int i = 0;
  uint64_t oldcnt = 0;
  // while (true)
  // {
  //   sleep(1);
  //   std::cout << "start" << std::endl;
  // }

  for (int index = 0; index < 2; index++)
  {
    auto thread = std::thread([&, index]() -> void {
      while (true)
      {
        kvsm::Request oReq;
        oReq.set_op(kvsm::Request_OpType_Put);
        oReq.set_key(std::to_string(i++));
        oReq.set_value(std::to_string(i++));
        std::string strReq;
        oReq.SerializeToString(&strReq);
        uint64_t id;

        auto iRet = node1.Propose(index, strReq, id);
        SPDLOG_INFO("Propose {} result: {}", i, iRet);
        g_cnt++;
      }
    });
    thread.detach();
  }
  while (true)
  {
    sleep(1);
    SPDLOG_ERROR("run {}", g_cnt - oldcnt);
    oldcnt = g_cnt;
  }
}