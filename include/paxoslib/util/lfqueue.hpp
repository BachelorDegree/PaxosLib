#pragma once
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/policies.hpp>
namespace paxoslib
{
namespace util
{
template <class T>
class LFQueue
{
public:
  LFQueue() : m_queue(1024)
  {
  }
  void push_back(const T &oItem)
  {
    Node oNode;
    oNode.value = new T{oItem};
    m_queue.push(oNode);
  }
  void push_back(T &&oItem)
  {
    Node oNode;
    oNode.value = new T{std::move(oItem)};
    m_queue.push(oNode);
  }
  bool pop_front(T &oItem)
  {
    Node oNode;
    bool bRet = m_queue.pop(oNode);
    if (!bRet)
    {
      return false;
    }
    oItem = std::move(*oNode.value);
    delete oNode.value;
    return true;
  }

private:
  struct Node
  {
    T *value;
  };
  boost::lockfree::queue<Node> m_queue;
};
}; // namespace util

}; // namespace paxoslib