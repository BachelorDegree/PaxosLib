#pragma once
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
namespace paxoslib
{
namespace util
{
class EPoll
{
public:
private:
  int m_fd;
  std::map<int, int> m_mapEvent;

public:
  EPoll(int size)
  {
    this->m_fd = epoll_create(size);
    assert(this->m_fd >= 0);
  }
  ~EPoll()
  {
    close(this->m_fd);
  }
  EPoll(const EPoll &) = delete;
  EPoll &operator=(const EPoll &) = delete;
  void WatchReadable(int fd)
  {
    this->AddWatchEvent(fd, EPOLLIN | EPOLLET);
  }
  void WatchWritable(int fd)
  {
    this->AddWatchEvent(fd, EPOLLOUT | EPOLLET);
  }
  void WatchDisconnect(int fd)
  {
    this->AddWatchEvent(fd, EPOLLHUP | EPOLLRDHUP);
  }
  void UnwatchReadable(int fd)
  {
    this->RemoveWatchEvent(fd, EPOLLIN | EPOLLET);
  }
  void UnwatchWritable(int fd)
  {
    this->RemoveWatchEvent(fd, EPOLLOUT | EPOLLET);
  }
  void UnwatchDisconnect(int fd)
  {
    this->RemoveWatchEvent(fd, EPOLLHUP | EPOLLRDHUP);
  }
  void AddWatchEvent(int fd, int to_set_event)
  {
    int event = 0;
    if (this->m_mapEvent.count(fd) != 0)
    {
      event = this->m_mapEvent[fd];
    }
    event |= to_set_event;
    this->Watch(fd, event);
  }
  void RemoveWatchEvent(int fd, int to_del_event)
  {
    int event = 0;
    if (this->m_mapEvent.count(fd) != 0)
    {
      event = this->m_mapEvent[fd];
    }
    event ^= to_del_event;
    this->Watch(fd, event);
  }
  void Unwatch(int fd)
  {
    this->Watch(fd, 0);
  }
  void Watch(int fd, int events)
  {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    if (events == 0)
    {
      if (this->m_mapEvent.count(fd) == 0)
      {
        return;
      }
      this->m_mapEvent.erase(fd);
      epoll_ctl(this->m_fd, EPOLL_CTL_DEL, fd, &event);
      return;
    }
    if (this->m_mapEvent.count(fd) == 0)
    {
      this->m_mapEvent[fd] = events;
      epoll_ctl(this->m_fd, EPOLL_CTL_ADD, fd, &event);
    }
    else
    {
      this->m_mapEvent[fd] = events;
      epoll_ctl(this->m_fd, EPOLL_CTL_MOD, fd, &event);
    }
  }
  int WatiEvents(epoll_event *events, int max_event, int timeout)
  {
    return epoll_wait(this->m_fd, events, max_event, timeout);
  }
};
}; // namespace util

}; // namespace paxoslib