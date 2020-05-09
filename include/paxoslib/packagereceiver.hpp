#pragma once
#include <cstdint>
namespace paxoslib
{
class Message;
};
namespace paxoslib::network
{
class PackageReceiver
{
public:
  virtual int OnPackage(char *pBuffer, uint32_t size) = 0;
  virtual ~PackageReceiver(){};
};
}; // namespace paxoslib::network