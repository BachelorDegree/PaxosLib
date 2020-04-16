#pragma once
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <map>
#include <set>
#include "paxoslib/util/lfqueue.hpp"
namespace paxoslib::network
{
class Channel
{
public:
  Channel(std::weak_ptr<Network> pNetwork, int fd);
  void EnqueueSendMessage(std::unique_ptr<char[]> pBuffer, uint32_t size);

  virtual void OnDisconnect();
  virtual void OnWritableOrTaskArrive();
  virtual void OnReadable();
  virtual uint64_t GetPeerId() const = 0;
  int GetFd() const;
  virtual ~Channel();

protected:
  struct ReceiveState
  {
    int state;
    uint32_t size;
    std::unique_ptr<char[]> pBuffer;
    uint32_t read;
  };
  struct SendState
  {
    int state;
    uint32_t size;
    std::unique_ptr<char[]> pBuffer;
    uint32_t write;
  };
  class QueueItem
  {
  public:
    uint32_t size;
    std::unique_ptr<char[]> pBuffer;
    uint32_t sent;
    QueueItem &operator=(QueueItem &&other)
    {
      if (this != &other)
      {
        this->pBuffer = std::move(other.pBuffer);
        this->size = other.size;
        this->sent = other.sent;
      }
      return *this;
    }
    QueueItem() {}
    QueueItem(QueueItem &&other)
    {
      *this = std::move(other);
    }
  };
  util::LFQueue<QueueItem> m_SendQueue;
  int m_fd;
  bool m_bHealthy;
  ReceiveState m_ReceiveState;
  SendState m_SendState;
  std::weak_ptr<Network> m_pNetwork;
};
class ChannelIncoming : public Channel
{
public:
  ChannelIncoming(std::weak_ptr<Network> pNetwork, int fd);
  virtual void OnReadable();
  virtual uint64_t GetPeerId() const;

private:
  uint64_t m_peer_id;
  int m_iPrepareState;
};
class ChannelOutgoing : public Channel
{
public:
  ChannelOutgoing(std::weak_ptr<Network> pNetwork, int fd, uint64_t peer_id);
  virtual void OnWritableOrTaskArrive();
  virtual uint64_t GetPeerId() const;

private:
  uint64_t m_peer_id;
  uint64_t m_my_id;
  int m_iPrepareState;
};
}; // namespace paxoslib::network