#pragma once
#include <string>
#include <spdlog/spdlog.h>
#include <rocksdb/db.h>
class KVClient
{
public:
  KVClient(const std::string &strPath)
  {
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, strPath, &db);
    assert(status.ok());
    SPDLOG_INFO("Rocksdb init success.");
  }
  ~KVClient()
  {
    if (db)
    {
      delete db;
    }
  }
  int Get(const std::string &strKey, std::string &strValue)
  {
    rocksdb::ReadOptions options;
    auto status = db->Get(options, strKey, &strValue);
    if (status.IsNotFound())
    {
      return 404;
    }
    if (status.ok() == false)
    {
      return -100;
    }
    return 0;
  }
  int Set(const std::string &strKey, const std::string &strValue)
  {
    rocksdb::WriteOptions options;
    auto status = db->Put(options, strKey, strValue);
    if (status.ok() == false)
    {
      return -100;
    }
    return 0;
  }
  int Del(const std::string &strKey)
  {
    rocksdb::WriteOptions options;
    auto status = db->Delete(options, strKey);
    if (status.ok() == false)
    {
      return -100;
    }
    return 0;
  }

private:
  rocksdb::DB *db = nullptr;
};