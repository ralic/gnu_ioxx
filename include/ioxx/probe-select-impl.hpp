#ifndef IOXX_PROBE_SELECT_IMPL_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_SELECT_IMPL_HPP_INCLUDED_2008_04_20

#include "socket.hpp"
#include "socket-event.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <sys/select.h>

inline std::ostream & operator<< (std::ostream & os, fd_set const & fds)
{
  for (int i( 0 ); i <= FD_SETSIZE; ++i)
    os << (FD_ISSET(i, &fds) ? '1' : '0');
    return os;
}

namespace ioxx
{
  class select_epoll_implementation
  {
  public:
    select_epoll_implementation(unsigned int /* size_hint */) : _max_fd(-1), _events(0u), _current(0)
    {
      FD_ZERO(&_req_read_fds);
      FD_ZERO(&_req_write_fds);
      FD_ZERO(&_req_except_fds);
    }

    void set(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(s <= FD_SETSIZE);
      if (request & ev_readable) { FD_SET(s, &_req_read_fds); }   else { FD_CLR(s, &_req_read_fds); }
      if (request & ev_writable) { FD_SET(s, &_req_write_fds); }  else { FD_CLR(s, &_req_write_fds); }
      if (request & ev_pridata)  { FD_SET(s, &_req_except_fds); } else { FD_CLR(s, &_req_except_fds); }
      if (request != ev_idle)
        _max_fd = std::max(_max_fd, s);
      else
        update_max_fd();
      IOXX_TRACE_MSG("select_probe: new _max_fd is " << _max_fd);
    }

    void modify(socket_t s, socket_event request)
    {
      set(s, request);
    }

    void unset(socket_t s)
    {
      set(s, ev_idle);
    }

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max());
    }

    bool pop_event(socket_t & s, socket_event & sev)
    {
      while (_events)
      {
        IOXX_TRACE_MSG("select::pop_event() has " << _events << " events to deliver; _max_fd = " << _max_fd << "; _current = " << _current);
        BOOST_ASSERT(_current <= _max_fd);
        s = _current++;
        BOOST_ASSERT(s >= 0);
        sev = ev_idle;
        if (FD_ISSET(s, &_recv_read_fds))   { --_events; sev |= ev_readable; }
        if (FD_ISSET(s, &_recv_write_fds))  { --_events; sev |= ev_writable; }
        if (FD_ISSET(s, &_recv_except_fds)) { --_events; sev |= ev_pridata; }
        if (sev != ev_idle) return true;
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(!_events);
      BOOST_ASSERT(timeout <= max_timeout());
      timeval tv = { timeout, 0 };
      if (_max_fd == -1)
      {
        select(0, NULL, NULL, NULL, &tv);
        return;
      }
      _recv_read_fds   = _req_read_fds;
      _recv_write_fds  = _req_write_fds;
      _recv_except_fds = _req_except_fds;
      int rc( select(_max_fd + 1, &_recv_read_fds, &_recv_write_fds, &_recv_except_fds, &tv) );
      IOXX_TRACE_MSG("select::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "select(2)");
        throw err;
      }
      _events  = static_cast<size_t>(rc);
      _current = 0;
    }

  private:
    fd_set   _req_read_fds, _req_write_fds, _req_except_fds;
    fd_set   _recv_read_fds, _recv_write_fds, _recv_except_fds;
    socket_t _max_fd;
    size_t   _events;
    socket_t _current;

    void update_max_fd()
    {
      while(_max_fd >= 0)
      {
        if (FD_ISSET(_max_fd, &_req_read_fds) || FD_ISSET(_max_fd, &_req_write_fds) || FD_ISSET(_max_fd, &_req_except_fds))
          break;
        else
          --_max_fd;
      }
    }
  };

} // namespace ioxx

#endif // IOXX_PROBE_SELECT_IMPL_HPP_INCLUDED_2008_04_20
