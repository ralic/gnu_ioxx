#ifndef IOXX_PROBE_EPOLL_IMPL_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_EPOLL_IMPL_HPP_INCLUDED_2008_04_20

#include "socket.hpp"
#include "socket-event.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <sys/epoll.h>

namespace ioxx
{
  class probe_epoll_implementation : private boost::noncopyable
  {
  public:
    probe_epoll_implementation(unsigned int size_hint) : _events_len(0u), _current(0u)
    {
      size_hint = std::min(size_hint, static_cast<unsigned int>(std::numeric_limits<int>::max()));
      _epoll_fd = throw_errno_if_minus1("create epoll socket", boost::bind(&epoll_create, static_cast<int>(size_hint)));
    }

    ~probe_epoll_implementation()
    {
      close(_epoll_fd, "close epoll socket");
    }

    void set(socket_t s, socket_event request, int op = EPOLL_CTL_ADD)
    {
      BOOST_ASSERT(s >= 0);
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = 0;
      if (request & ev_readable) ev.events |= EPOLLIN;
      if (request & ev_writable) ev.events |= EPOLLOUT;
      if (request & ev_pridata)  ev.events |= EPOLLPRI;
      throw_errno_if_minus1("epoll", boost::bind(&epoll_ctl, _epoll_fd, op, s, &ev));
    }

    void modify(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      set(s, request, EPOLL_CTL_MOD);
    }

    void unset(socket_t s)
    {
      BOOST_ASSERT(s >= 0);
      set(s, ev_idle, EPOLL_CTL_DEL);
    }

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000);
    }

    bool pop_event(socket_t & s, socket_event & sev)
    {
      IOXX_TRACE_MSG("epoll::pop_event() has " << _events_len << " events to deliver");
      if (!_events_len) return false;
      s = _events[_current].data.fd;
      BOOST_ASSERT(s >= 0);
      sev = ev_idle;
      if (_events[_current].events & EPOLLIN)  sev |= ev_readable;
      if (_events[_current].events & EPOLLOUT) sev |= ev_writable;
      if (_events[_current].events & EPOLLPRI) sev |= ev_pridata;
      BOOST_ASSERT(sev != ev_idle);
      if (--_events_len) ++_current; else _current = 0;
      return true;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_events_len);
      int const rc( epoll_wait(_epoll_fd
                              , _events, sizeof(_events) / sizeof(epoll_event)
                              , static_cast<int>(timeout) * 1000
                              ));
      IOXX_TRACE_MSG("epoll::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "epoll_wait(2)");
        throw err;
      }
      _events_len = static_cast<size_t>(rc);
      _current    = 0u;
    }

  private:
    socket_t    _epoll_fd;
    epoll_event _events[128];
    size_t      _events_len;
    size_t      _current;
  };

} // namespace ioxx

#endif // IOXX_PROBE_EPOLL_IMPL_HPP_INCLUDED_2008_04_20
