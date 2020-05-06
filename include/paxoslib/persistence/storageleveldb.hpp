#pragma once
#include <cstdint>
#include <leveldb/db.h>
#include "paxoslib/persistence/storage.hpp"
namespace paxoslib
{
namespace persistence
{
class StorageLeveldb : public Storage
{
public:
  StorageLeveldb(const std::string &strPath);
  virtual int LoadState(uint64_t id, paxoslib::persistence::StateProto &oState) const;
  virtual int SaveState(uint64_t id, const paxoslib::persistence::StateProto &oState);
  virtual int GetMaxInstanceId(uint64_t &id);
  virtual ~StorageLeveldb();

private:
  int Save(uint64_t id, const std::string &strValue);
  int Load(uint64_t id, std::string &strValue) const;
  leveldb::DB *db;
};
}; // namespace persistence
}; // namespace paxoslib