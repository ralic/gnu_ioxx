/*
 * Copyright (c) 2008 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Copying and distribution of this file, with or without modification, are
 * permitted in any medium without royalty provided the copyright notice and
 * this notice are preserved.
 */

#ifndef IOXX_DETAIL_SELECT_HPP_INCLUDED_2008_05_19
#define IOXX_DETAIL_SELECT_HPP_INCLUDED_2008_05_19

#include <ioxx/socket.hpp>
#include <ioxx/signal.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <iosfwd>
#include <sys/select.h>

namespace ioxx { namespace detail
{
  typedef unsigned int seconds_t;

  /**
   * \internal
   *
   * \brief I/O demultiplexer implementation based on \c select(2).
   *
   * \sa http://www.opengroup.org/onlinepubs/009695399/functions/select.html
   */
  class select : private boost::noncopyable
  {
  public:
    class socket : public system_socket
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
      friend inline std::ostream & operator<< (std::ostream & os, event_set ev)
      {
        if (ev == no_events) os << "None";
        if (ev & readable)   os << "Read";
        if (ev & writable)   os << "Write";
        if (ev & pridata)    os << "Pridata";
        return os;
      }

      socket(select & demux, native_socket_t sock, event_set ev = no_events) : system_socket(sock), _select(demux)
      {
        BOOST_ASSERT(sock >= 0);
        BOOST_ASSERT(sock <= FD_SETSIZE);
        request(ev);
      }

      ~socket()
      {
        request(no_events);
      }

      void request(event_set ev)
      {
        native_socket_t const s( as_native_socket_t() );
        if (ev & readable) { FD_SET(s, &_select._req_read_fds); }   else { FD_CLR(s, &_select._req_read_fds); }
        if (ev & writable) { FD_SET(s, &_select._req_write_fds); }  else { FD_CLR(s, &_select._req_write_fds); }
        if (ev & pridata)  { FD_SET(s, &_select._req_except_fds); } else { FD_CLR(s, &_select._req_except_fds); }
        if (ev != no_events)
        {
          _select._max_fd = std::max(_select._max_fd, s);
        }
        else if (s == _select._max_fd)
        {
          while(_select._max_fd >= 0)
          {
            if (  FD_ISSET(_select._max_fd, &_select._req_read_fds)
               || FD_ISSET(_select._max_fd, &_select._req_write_fds)
               || FD_ISSET(_select._max_fd, &_select._req_except_fds)
               )
              break;
            else
              --_select._max_fd;
          }
          LOGXX_TRACE("select: new _max_fd is " << _select._max_fd);
        }
        else
          BOOST_ASSERT(s < _select._max_fd);
      }

    protected:
      select & context() { return _select; }

    private:
      select & _select;
    };

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max());
    }

    select() : _max_fd(-1), _current(0), _n_events(0u)
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.select(" + detail::show(this) + ')');
      FD_ZERO(&_req_read_fds);
      FD_ZERO(&_req_write_fds);
      FD_ZERO(&_req_except_fds);
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, socket::event_set & ev)
    {
      while (_n_events)
      {
        LOGXX_TRACE("pop_event() has " << _n_events << " events to deliver; _max_fd = " << _max_fd << "; _current = " << _current);
        BOOST_ASSERT(_current <= _max_fd);
        sock = _current++;
        BOOST_ASSERT(sock >= 0);
        ev = socket::no_events;
        if (FD_ISSET(sock, &_recv_read_fds))   { --_n_events; ev |= socket::readable; }
        if (FD_ISSET(sock, &_recv_write_fds))  { --_n_events; ev |= socket::writable; }
        if (FD_ISSET(sock, &_recv_except_fds)) { --_n_events; ev |= socket::pridata; }
        if (ev != socket::no_events)
        {
          LOGXX_TRACE("deliver events " << ev << " on socket " << sock);
          return true;
        }
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
      if (_max_fd == -1)
      {
#if defined(IOXX_HAVE_PSELECT) && IOXX_HAVE_PSELECT
        timespec const to = { timeout, 0 };
        sigset_t unblock_all;
        throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
        ::pselect(0, NULL, NULL, NULL, &to, &unblock_all);
#else
        timeval tv = { timeout, 0 };
        unblock_signals signal_scope;
        ::select(0, NULL, NULL, NULL, &tv);
#endif
        return;
      }
      _recv_read_fds   = _req_read_fds;
      _recv_write_fds  = _req_write_fds;
      _recv_except_fds = _req_except_fds;
#if defined(IOXX_HAVE_PSELECT) && IOXX_HAVE_PSELECT
      timespec const to = { timeout, 0 };
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
      int const rc( ::pselect(_max_fd + 1, &_recv_read_fds, &_recv_write_fds, &_recv_except_fds, &to, &unblock_all) );
#else
      int rc;
      {
        timeval tv = { timeout, 0 };
        unblock_signals signal_scope;
        rc = ::select(_max_fd + 1, &_recv_read_fds, &_recv_write_fds, &_recv_except_fds, &tv);
      }
#endif
      LOGXX_TRACE("wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        system_error err(errno, "select(2)");
        throw err;
      }
      _n_events  = static_cast<size_t>(rc);
      _current = 0;
    }

  protected:
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);

  private:
    fd_set              _req_read_fds, _req_write_fds, _req_except_fds;
    fd_set              _recv_read_fds, _recv_write_fds, _recv_except_fds;
    native_socket_t     _max_fd;
    native_socket_t     _current;
    size_t              _n_events;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_SELECT_HPP_INCLUDED_2008_05_19
