#include <cstdio>
#include <cstdint>
#include <cinttypes>
#include <string>
#include <leveldb/db.h>
#include <leveldb/comparator.h>
#include <assert.h>
#include "paxoslib/proto/common.pb.h"
#include "paxoslib/persistence/storageleveldb.hpp"
#include "paxoslib/persistence/uint64comparator.hpp"
namespace paxoslib
{
namespace persistence
{
std::string GenKey(uint64_t key)
{
  std::string str;
  str.append((char *)&key, sizeof(key));
  return str;
}
StorageLeveldb::StorageLeveldb(const std::string &strPath)
{
  static Uint64Compartor oCmp;
  leveldb::Options options;
  options.create_if_missing = true;
  options.comparator = &oCmp;
  leveldb::Status status = leveldb::DB::Open(options, strPath, &db);
  assert(status.ok());
}
int StorageLeveldb::LoadState(uint64_t id, paxoslib::StateProto &oState) const
{
  std::string strBuffer;
  int iRet = Load(id, strBuffer);
  if (iRet != 0)
  {
    return iRet;
  }
  oState.ParseFromString(strBuffer);
  return 0;
}
int StorageLeveldb::SaveState(uint64_t id, const paxoslib::StateProto &oState)
{
  std::string strBuffer;
  oState.SerializeToString(&strBuffer);
  int iRet = Save(id, strBuffer);
  if (iRet != 0)
  {
    return iRet;
  }
  return 0;
}
int StorageLeveldb::GetMaxInstanceId(uint64_t &id)
{
  id = 0;
  auto pIterator = db->NewIterator(leveldb::ReadOptions());
  pIterator->SeekToLast();
  if (pIterator->Valid() == false)
  {
    return 0;
  }
  if (pIterator->key().size() != sizeof(uint64_t))
  {
    return 1;
  }
  memcpy(&id, pIterator->key().data(), pIterator->key().size());
  return 0;
}
int StorageLeveldb::Save(uint64_t id, const std::string &strValue)
{
  leveldb::Status s = db->Put(leveldb::WriteOptions(), GenKey(id), strValue);
  if (s.ok())
  {
    return 0;
  }
  return -1;
}
int StorageLeveldb::Load(uint64_t id, std::string &strValue) const
{
  strValue.clear();
  leveldb::Status s = db->Get(leveldb::ReadOptions(), GenKey(id), &strValue);
  if (s.ok())
  {
    return 0;
  }
  if (s.IsNotFound())
  {
    return 404;
  }
  return -1;
}
StorageLeveldb::~StorageLeveldb()
{
  if (db)
  {
    delete db;
  }
}
}; // namespace persistence
}; // namespace paxoslib