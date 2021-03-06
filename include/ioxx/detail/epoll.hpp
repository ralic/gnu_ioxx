/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_DETAIL_EPOLL_HPP_INCLUDED_2010_02_23
#define IOXX_DETAIL_EPOLL_HPP_INCLUDED_2010_02_23

#include <ioxx/socket.hpp>
#include <ioxx/signal.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <limits>
#include <iosfwd>
#include <sys/epoll.h>

namespace ioxx { namespace detail
{
  typedef unsigned int seconds_t;

  /**
   * \internal
   *
   * \brief I/O demultiplexer implementation based on \c epoll(7).
   *
   * \sa http://www.kernel.org/doc/man-pages/online/pages/man7/epoll.7.html
   */
  class epoll : private boost::noncopyable
  {
  public:
    class socket : public system_socket
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

      socket(epoll & demux, native_socket_t sock, event_set ev = no_events) : system_socket(sock), _epoll(demux)
      {
        BOOST_ASSERT(sock >= 0);
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = ev;
        LOGXX_MSG_TRACE(context().LOGXX_SCOPE_NAME, "register socket " << e.data.fd  << " events " << ev);
        throw_errno_if_minus1("add socket into epoll", boost::bind(boost::type<int>(), &epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_ADD, as_native_socket_t(), &e));
      }

      ~socket()
      {
        LOGXX_MSG_TRACE(context().LOGXX_SCOPE_NAME, "unregister " << *this);
        if (close_on_destruction()) return;
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = 0;
        throw_errno_if_minus1("del socket from epoll", boost::bind(boost::type<int>(), &epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_DEL, as_native_socket_t(), &e));
      }

      void request(event_set ev)
      {
        epoll_event e;
        e.data.fd = as_native_socket_t();
        e.events  = ev;
        LOGXX_MSG_TRACE(context().LOGXX_SCOPE_NAME, "modify socket " << e.data.fd << " events " << ev);
        throw_errno_if_minus1("modify socket in epoll", boost::bind(boost::type<int>(), &epoll_ctl, _epoll._epoll_fd, EPOLL_CTL_MOD, as_native_socket_t(), &e));
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
      _epoll_fd = throw_errno_if_minus1("create epoll socket", boost::bind(boost::type<int>(), &epoll_create, static_cast<int>(size_hint)));
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.epoll(" + detail::show(_epoll_fd) + ')');
    }

    ~epoll()
    {
      throw_errno_if_minus1("close epoll socket", boost::bind(boost::type<int>(), &::close, _epoll_fd));
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, socket::event_set & ev)
    {
      LOGXX_TRACE("pop_event() has " << _n_events << " events to deliver");
      if (!_n_events) return false;
      sock = _events[_current].data.fd;
      ev   = static_cast<socket::event_set>(_events[_current].events);
      ev  |= ev & EPOLLRDNORM ? socket::readable : socket::no_events; // weird, redundant extensions
      ev  |= ev & EPOLLRDBAND ? socket::pridata  : socket::no_events;
      ev  |= ev & EPOLLWRNORM ? socket::writable : socket::no_events;
      ev  &= socket::readable | socket::writable | socket::pridata;
      BOOST_ASSERT(ev != socket::no_events);
      --_n_events; ++_current;
      LOGXX_TRACE("deliver events " << ev << " on socket " << sock);
      return true;
   }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
#if defined IOXX_HAVE_EPOLL_PWAIT && IOXX_HAVE_EPOLL_PWAIT
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
      int const rc( epoll_pwait( _epoll_fd
                               , _events, sizeof(_events) / sizeof(epoll_event)
                               , static_cast<int>(timeout) * 1000
                               , &unblock_all
                               ));
#else
      int rc;
      {
        signal_unblock signal_scope;
        rc = epoll_wait( _epoll_fd
                       , _events, sizeof(_events) / sizeof(epoll_event)
                       , static_cast<int>(timeout) * 1000
                       );
      }
#endif
      LOGXX_TRACE("wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        system_error err(errno, "epoll_wait(2)");
        throw err;
      }
      _n_events = static_cast<size_t>(rc);
      _current    = 0u;
    }

  protected:
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);

  private:
    native_socket_t     _epoll_fd;
    epoll_event         _events[128];
    size_t              _n_events;
    size_t              _current;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_EPOLL_HPP_INCLUDED_2010_02_23
