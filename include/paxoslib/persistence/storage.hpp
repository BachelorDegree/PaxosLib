#pragma once
#include <cstdint>
namespace paxoslib
{
namespace persistence
{
class StateProto;
class Storage
{
public:
  Storage() {}
  virtual int LoadState(uint64_t id, paxoslib::persistence::StateProto &oState) const = 0;
  virtual int SaveState(uint64_t id, const paxoslib::persistence::StateProto &oState) = 0;
  virtual int GetMaxInstanceId(uint64_t &id) = 0;
  virtual ~Storage() {}

private:
};
}; // namespace persistence
}; // namespace paxoslib