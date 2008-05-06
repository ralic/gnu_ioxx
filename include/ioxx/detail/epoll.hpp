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

#ifndef IOXX_DETAIL_EPOLL_HPP_INCLUDED_2008_04_20
#define IOXX_DETAIL_EPOLL_HPP_INCLUDED_2008_04_20

#include <ioxx/detail/socket.hpp>
#include <ioxx/detail/signals.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <iosfwd>
#include <sys/epoll.h>

namespace ioxx { namespace detail
{
  typedef unsigned int seconds_t;

  /**
   * I/O demultiplexer implementation based on \c epoll(7).
   */
  class epoll : private boost::noncopyable
  {
  public:
    class socket : public detail::socket
    {
    public:
      enum event_set
        { no_events = 0
        , readable  = EPOLLIN
        , writable  = EPOLLOUT
        , pridata   = EPOLLPRI
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

      socket(epoll & demux, native_socket_t sock, event_set ev = no_events) : detail::socket(sock), _epoll(demux)
      {
        BOOST_ASSERT(sock >= 0);
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = ev;
        throw_errno_if_minus1("add socket into epoll", boost::bind(&epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_ADD, as_native_socket_t(), &e));
      }

      ~socket()
      {
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = 0;
        throw_errno_if_minus1("del socket from epoll", boost::bind(&epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_DEL, as_native_socket_t(), &e));
      }

      void request(event_set ev)
      {
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = ev;
        throw_errno_if_minus1("modify socket in epoll", boost::bind(&epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_MOD, as_native_socket_t(), &e));
      }

    protected:
      epoll & context() { return _epoll; }

    private:
      epoll & _epoll;
    };

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000);
    }

    explicit epoll(unsigned int size_hint = 128u) : _n_events(0u), _current(0u)
    {
      size_hint = std::min(size_hint, static_cast<unsigned int>(std::numeric_limits<int>::max()));
      _epoll_fd = throw_errno_if_minus1("create epoll socket", boost::bind(&epoll_create, static_cast<int>(size_hint)));
    }

    ~epoll()
    {
      throw_errno_if_minus1("close epoll socket", boost::bind(&::close, _epoll_fd));
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, socket::event_set & ev)
    {
      IOXX_TRACE_MSG("epoll::pop_event() has " << _n_events << " events to deliver");
      if (!_n_events) return false;
      sock = _events[_current].data.fd;
      ev   = static_cast<socket::event_set>(_events[_current].events);
      BOOST_ASSERT(ev != socket::no_events);
      --_n_events; ++_current;
      return true;
   }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
#if defined(IOXX_HAVE_EPOLL_PWAIT) && IOXX_HAVE_EPOLL_PWAIT
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(&::sigemptyset, &unblock_all));
      int const rc( epoll_pwait( _epoll_fd
                               , _events, sizeof(_events) / sizeof(epoll_event)
                               , static_cast<int>(timeout) * 1000
                               , &unblock_all
                               ));
#else
      int rc;
      {
        unblock_signals signal_scope;
        rc = epoll_wait( _epoll_fd
                       , _events, sizeof(_events) / sizeof(epoll_event)
                       , static_cast<int>(timeout) * 1000
                       );
      }
#endif
      IOXX_TRACE_MSG("epoll::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        system_error err(errno, "epoll_wait(2)");
        throw err;
      }
      _n_events = static_cast<size_t>(rc);
      _current    = 0u;
    }

  private:
    native_socket_t     _epoll_fd;
    epoll_event         _events[128];
    size_t              _n_events;
    size_t              _current;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_EPOLL_HPP_INCLUDED_2008_04_20
