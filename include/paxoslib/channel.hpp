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
  virtual void OnDisconnect() = 0;
  virtual void OnWritableOrTaskArrive() = 0;
  virtual void OnReadable() = 0;
  virtual uint16_t GetPeerId() const = 0;
  int GetFd() const
  {
    return m_fd;
  };
  virtual ~Channel();

protected:
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
  int m_fd;
  bool m_bHealthy;
  std::weak_ptr<Network> m_pNetwork;
  util::LFQueue<QueueItem> m_SendQueue;
};
class ChannelSelf : public Channel
{
public:
  ChannelSelf(std::weak_ptr<Network> pNetwork, int fd, uint16_t peer_id);
  virtual void OnDisconnect();
  virtual void OnWritableOrTaskArrive();
  virtual void OnReadable();
  virtual uint16_t GetPeerId() const;

private:
  uint16_t m_peer_id;
};
class ChannelStream : public Channel
{
public:
  ChannelStream(std::weak_ptr<Network> pNetwork, int fd);
  virtual void OnDisconnect();
  virtual void OnWritableOrTaskArrive();
  virtual void OnReadable();
  virtual uint16_t GetPeerId() const = 0;

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
  ReceiveState m_ReceiveState;
  SendState m_SendState;
};
class ChannelIncoming : public ChannelStream
{
public:
  ChannelIncoming(std::weak_ptr<Network> pNetwork, int fd);
  virtual void OnReadable();
  virtual uint16_t GetPeerId() const;

private:
  uint16_t m_peer_id;
  int m_iPrepareState;
};
class ChannelOutgoing : public ChannelStream
{
public:
  ChannelOutgoing(std::weak_ptr<Network> pNetwork, int fd, uint16_t peer_id);
  virtual void OnWritableOrTaskArrive();
  virtual uint16_t GetPeerId() const;

private:
  uint16_t m_peer_id;
  uint64_t m_my_id;
  int m_iPrepareState;
};
}; // namespace paxoslib::network