#pragma once
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <map>
#include <set>
namespace paxoslib::network
{
class Channel
{
public:
  struct QueueItem
  {
    uint32_t size;
    char *pBuffer;
  };
  Channel(Network *pNetwork, int fd, uint64_t peer_id);
  bool IsChannelHealthy() const;
  ssize_t Send(const void *__buf, size_t __n, int __flags);
  ssize_t Recv(void *__buf, size_t __n, int __flags);
  void EnqueueSendMessage(char *pBuffer, uint32_t size);
  void SendWorker();
  void ReceiveWorker();
  uint64_t GetPeerId() const;

private:
  int m_fd;
  int m_peer_id;
  bool m_bHealthy;
  std::queue<QueueItem> m_SendQueue;
  Network *m_pNetwork;
  std::thread m_SendThread;
  std::thread m_ReceiveThread;
};
}; // namespace paxoslib::network