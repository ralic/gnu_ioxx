#ifndef IOXX_DEMUX_SELECT_HPP_INCLUDED_2008_04_20
#define IOXX_DEMUX_SELECT_HPP_INCLUDED_2008_04_20

#include "ioxx/socket.hpp"
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <sys/select.h>

namespace ioxx { namespace demux
{
  typedef unsigned int seconds_t;

  class select : private boost::noncopyable
  {
  public:
    class socket : public ioxx::socket
    {
    public:
      enum event_set
        { no_events = 0
        , readable  = 1 << 0
        , writable  = 1 << 1
        , pridata   = 1 << 2
        };

      friend inline event_set & operator|= (event_set & lhs, event_set rhs) { return lhs = (event_set)((int)(lhs) | (int)(rhs)); }
      friend inline event_set   operator|  (event_set   lhs, event_set rhs) { return lhs |= rhs; }
      friend inline event_set & operator&= (event_set & lhs, event_set rhs) { return lhs = (event_set)((int)(lhs) & (int)(rhs)); }
      friend inline event_set   operator&  (event_set   lhs, event_set rhs) { return lhs &= rhs; }

      socket(select & demux, native_socket_t sock, event_set ev) : ioxx::socket(sock), _demux(demux)
      {
        BOOST_ASSERT(*this);
        BOOST_ASSERT(sock <= FD_SETSIZE);
        request(ev);
      }

      ~socket()
      {
        request(no_events);
      }

      void request(event_set ev)
      {
        BOOST_ASSERT(*this);
        native_socket_t const s( as_native_socket_t() );
        if (ev & readable) { FD_SET(s, &_demux._req_read_fds); }   else { FD_CLR(s, &_demux._req_read_fds); }
        if (ev & writable) { FD_SET(s, &_demux._req_write_fds); }  else { FD_CLR(s, &_demux._req_write_fds); }
        if (ev & pridata)  { FD_SET(s, &_demux._req_except_fds); } else { FD_CLR(s, &_demux._req_except_fds); }
        if (ev != no_events)
        {
          _demux._max_fd = std::max(_demux._max_fd, s);
        }
        else if (s == _demux._max_fd)
        {
          while(_demux._max_fd >= 0)
          {
            if (  FD_ISSET(_demux._max_fd, &_demux._req_read_fds)
               || FD_ISSET(_demux._max_fd, &_demux._req_write_fds)
               || FD_ISSET(_demux._max_fd, &_demux._req_except_fds)
               )
              break;
            else
              --_demux._max_fd;
          }
          IOXX_TRACE_MSG("select: new _max_fd is " << _demux._max_fd);
        }
        else
          BOOST_ASSERT(s < _demux._max_fd);
      }

    private:           // those methods from ioxx::socket don't work for us yet
      void            swap(socket & other);
      void            reset(native_socket_t s);
      native_socket_t release();

    private:
      select & _demux;
    };

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max());
    }

    select(unsigned int /* size_hint */ = 0u) : _max_fd(-1), _current(0), _n_events(0u)
    {
      FD_ZERO(&_req_read_fds);
      FD_ZERO(&_req_write_fds);
      FD_ZERO(&_req_except_fds);
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, socket::event_set & ev)
    {
      while (_n_events)
      {
        IOXX_TRACE_MSG("select::pop_event() has " << _n_events << " events to deliver; _max_fd = " << _max_fd << "; _current = " << _current);
        BOOST_ASSERT(_current <= _max_fd);
        sock = _current++;
        BOOST_ASSERT(sock >= 0);
        ev = socket::no_events;
        if (FD_ISSET(sock, &_recv_read_fds))   { --_n_events; ev |= socket::readable; }
        if (FD_ISSET(sock, &_recv_write_fds))  { --_n_events; ev |= socket::writable; }
        if (FD_ISSET(sock, &_recv_except_fds)) { --_n_events; ev |= socket::pridata; }
        if (ev != socket::no_events) return true;
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
      timeval tv = { timeout, 0 };
      if (_max_fd == -1)
      {
        ::select(0, NULL, NULL, NULL, &tv);
        return;
      }
      _recv_read_fds   = _req_read_fds;
      _recv_write_fds  = _req_write_fds;
      _recv_except_fds = _req_except_fds;
      int const rc( ::select(_max_fd + 1, &_recv_read_fds, &_recv_write_fds, &_recv_except_fds, &tv) );
      IOXX_TRACE_MSG("select::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "select(2)");
        throw err;
      }
      _n_events  = static_cast<size_t>(rc);
      _current = 0;
    }

  private:
    fd_set              _req_read_fds, _req_write_fds, _req_except_fds;
    fd_set              _recv_read_fds, _recv_write_fds, _recv_except_fds;
    native_socket_t     _max_fd;
    native_socket_t     _current;
    size_t              _n_events;
  };

}} // namespace ioxx::demux

#endif // IOXX_DEMUX_SELECT_HPP_INCLUDED_2008_04_20
