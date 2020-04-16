#include <unistd.h>
#include "paxoslib/proto/config.pb.h"
#include "paxoslib/instance.hpp"
#include "paxoslib/network.hpp"
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
  sleep(1);
  paxoslib::Instance oInstance1{oConfig1, network1};
  paxoslib::Instance oInstance2{oConfig2, network2};
  paxoslib::Instance oInstance3{oConfig3, network3};
  sleep(1);
  oInstance1.Propose("test");
  while (true)
  {
    sleep(1);
  }
}