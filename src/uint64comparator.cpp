#include <cassert>
#include <cstring>
#include <leveldb/db.h>
#include "paxoslib/persistence/uint64comparator.hpp"
namespace paxoslib
{
namespace persistence
{
// Three-way comparison.  Returns value:
//   < 0 iff "a" < "b",
//   == 0 iff "a" == "b",
//   > 0 iff "a" > "b"
int Uint64Compartor::Compare(const leveldb::Slice &a, const leveldb::Slice &b) const
{
  assert(b.size() == sizeof(uint64_t));
  assert(a.size() == sizeof(uint64_t));
  uint64_t a_v;
  uint64_t b_v;
  memcpy(&a_v, a.data(), sizeof(a_v));
  memcpy(&b_v, b.data(), sizeof(b_v));
  if (a_v == b_v)
  {
    return 0;
  }
  if (a_v < b_v)
  {
    return -1;
  }
  return 1;
}

// The name of the comparator.  Used to check for comparator
// mismatches (i.e., a DB created with one comparator is
// accessed using a different comparator.
//
// The client of this package should switch to a new name whenever
// the comparator implementation changes in a way that will cause
// the relative ordering of any two keys to change.
//
// Names starting with "leveldb." are reserved and should not be used
// by any clients of this package.
const char *Uint64Compartor::Name() const
{
  return "Uint64Compartor";
}

// Advanced functions: these are used to reduce the space requirements
// for internal data structures like index blocks.

// If *start < limit, changes *start to a short string in [start,limit).
// Simple comparator implementations may return with *start unchanged,
// i.e., an implementation of this method that does nothing is correct.
void Uint64Compartor::FindShortestSeparator(std::string *start,
                                            const leveldb::Slice &limit) const
{
}

// Changes *key to a short string >= *key.
// Simple comparator implementations may return with *key unchanged,
// i.e., an implementation of this method that does nothing is correct.
void Uint64Compartor::FindShortSuccessor(std::string *key) const {}
}; // namespace persistence
}; // namespace paxoslib