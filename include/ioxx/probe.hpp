#ifndef IOXX_PROBE_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <map>
#include <algorithm>
#include <limits>
#include <sys/epoll.h>

namespace ioxx
{
  typedef int socket_t;
  typedef unsigned int seconds_t;

  enum socket_event
    { ev_error     = 1 << 0
    , ev_idle      = ev_error
    , ev_readable  = 1 << 1
    , ev_writable  = 1 << 2
    , ev_pridata   = 1 << 3
    };

  inline bool is_error(socket_event st) { return !st; }

  class probe : private boost::noncopyable
  {
    static short to_events(socket_event sev)
    {
      short ev( 0 );
      if (sev & ev_readable)     ev |= EPOLLIN;
      if (sev & ev_writable)     ev |= EPOLLOUT;
      if (sev & ev_pridata)      ev |= EPOLLPRI;
      return ev;
    }

  public:
    typedef boost::function<void (socket_event)>        handler;
    typedef std::map<socket_t,handler>                  handler_map;

    explicit probe(unsigned int size_hint = 512u)
    {
      BOOST_ASSERT(ev_error == 0);
      size_hint = std::min(size_hint, static_cast<unsigned int>(std::numeric_limits<int>::max()));
      _epoll_fd = throw_errno_if_minus1<socket_t>( "epoll_create(2)"
                                                 , boost::bind(&epoll_create, static_cast<int>(size_hint))
                                                 );
    }

    ~probe()
    {
      throw_errno_if_minus1_<int>("close() epoll socket", boost::bind(&close, _epoll_fd));
    }

    void insert(socket_t s, handler const & f, socket_event request = ev_idle)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) == _handlers.end());
      try
      {
        epoll_event ev;
        ev.data.fd = s;
        ev.events  = to_events(request);
        _handlers[s] = f;
        throw_errno_if_minus1_<int>( "registering new socket"
                                   , boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_ADD, s, &ev)
                                   );
      }
      catch(...)
      {
        _handlers.erase(s);
        throw;
      }
    }

    void modify(socket_t s, handler const & f)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) == _handlers.end());
      _handlers[s] = f;
    }

    void modify(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = to_events(request);
      throw_errno_if_minus1_<int>( "modifying socket events"
                                 , boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_MOD, s, &ev)
                                 );
    }

    void modify(socket_t s, handler f, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
      handler & h( _handlers[s] );
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = to_events(request);
      throw_errno_if_minus1_<int>( "modifying socket events"
                                 , boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_MOD, s, &ev)
                                 );
      std::swap(f, h);
    }

    void erase(socket_t s)
    {
      BOOST_ASSERT(s >= 0);
      handler_map::iterator const i( _handlers.find(s) );
      _handlers.erase(i);
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = 0;
      throw_errno_if_minus1_<int>( "unregistering socket"
                                 , boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_DEL, s, &ev)
                                 );
    }

    bool empty() const { return _handlers.empty(); }

    void run(seconds_t timeout)
    {
      timeout = std::min(timeout, static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000));
      epoll_event ev;
      int const rc( epoll_wait(_epoll_fd, &ev, 1, static_cast<int>(timeout) * 1000) );
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "epoll_wait(2)");
        throw err;
      }
      BOOST_ASSERT(rc == 1);
      handler_map::iterator const i( _handlers.find(ev.data.fd) );
      BOOST_ASSERT(i != _handlers.end());
      int sev( ev_error );
      if (ev.events & EPOLLIN)  sev |= ev_readable;
      if (ev.events & EPOLLOUT) sev |= ev_writable;
      if (ev.events & EPOLLPRI) sev |= ev_pridata;
      i->second(static_cast<socket_event>(sev));
    }

  private:
    socket_t    _epoll_fd;
    handler_map _handlers;
  };

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED_2008_04_20
