#ifndef IOXX_PROBE_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/assert.hpp>
#include <map>
#include <algorithm>
#include <limits>

#if defined(IOXX_HAVE_EPOLL)
#  include <sys/epoll.h>
#elif defined(IOXX_HAVE_SELECT)
#  include <sys/select.h>
#else
#  error "No I/O probe implementation available for this platform."
#endif

namespace ioxx
{
  typedef int socket_t;
  typedef unsigned int seconds_t;

  enum socket_event
    { ev_idle      = 0
    , ev_readable  = 1 << 1
    , ev_writable  = 1 << 2
    , ev_pridata   = 1 << 3
    };

  inline socket_event & operator|= (socket_event & lhs, socket_event rhs) { return lhs = (socket_event)((int)(lhs) | (int)(rhs)); }
  inline socket_event   operator|  (socket_event   lhs, socket_event rhs) { return lhs |= rhs; }
  inline socket_event & operator&= (socket_event & lhs, socket_event rhs) { return lhs = (socket_event)((int)(lhs) & (int)(rhs)); }
  inline socket_event   operator&  (socket_event   lhs, socket_event rhs) { return lhs &= rhs; }

  class probe : private boost::noncopyable
  {
  public:
    typedef boost::function<void (socket_event)>        handler;
    typedef std::map<socket_t,handler>                  handler_map;
    typedef handler_map::iterator                       iterator;

    explicit probe(unsigned int size_hint = 512u)
    {
#if defined(IOXX_HAVE_EPOLL)
      size_hint = std::min(size_hint, static_cast<unsigned int>(std::numeric_limits<int>::max()));
      _epoll_fd = throw_errno_if_minus1<socket_t>("epoll_create(2)", boost::bind(&epoll_create, static_cast<int>(size_hint)));
#elif defined(IOXX_HAVE_SELECT)
      FD_ZERO(&_read_fds);
      FD_ZERO(&_write_fds);
      FD_ZERO(&_except_fds);
#endif
    }

    ~probe()
    {
#if defined(IOXX_HAVE_EPOLL)
      throw_errno_if_minus1_("close() epoll socket", boost::bind(&close, _epoll_fd));
#elif defined(IOXX_HAVE_SELECT)
      /* */
#endif
    }

    void set(socket_t s, handler f, socket_event request = ev_idle)
    {
      BOOST_ASSERT(s >= 0);
      std::pair<iterator,bool> const r( _handlers.insert(std::make_pair(s, f)) );
      if (!r.second) std::swap(r.first->second, f);
#if defined(IOXX_HAVE_EPOLL)
      try
      {
        epoll_event ev;
        ev.data.fd = s;
        ev.events  = to_events(request);
        throw_errno_if_minus1_( "probe::set()"
                              , boost::bind(&epoll_ctl, _epoll_fd, r.second ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, s, &ev)
                              );
      }
      catch(...)
      {
        if (r.second)   _handlers.erase(r.first);
        else            std::swap(r.first->second, f);
        throw;
      }
#elif defined(IOXX_HAVE_SELECT)
      if (request & ev_readable) { FD_SET(s, &_read_fds); }   else { FD_CLR(s, &_read_fds); }
      if (request & ev_writable) { FD_SET(s, &_write_fds); }  else { FD_CLR(s, &_write_fds); }
      if (request & ev_pridata)  { FD_SET(s, &_except_fds); } else { FD_CLR(s, &_except_fds); }
#endif
    }

    void unset(socket_t s)
    {
      BOOST_ASSERT(s >= 0);
      if (!_handlers.erase(s)) return;
#if defined(IOXX_HAVE_EPOLL)
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = 0;
      throw_errno_if_minus1_("probe::unset()", boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_DEL, s, &ev));
#elif defined(IOXX_HAVE_SELECT)
      FD_CLR(s, &_read_fds);
      FD_CLR(s, &_write_fds);
      FD_CLR(s, &_except_fds);
#endif
    }

    void modify(socket_t s, handler const & f)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
      _handlers[s] = f;
    }

    void modify(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
#if defined(IOXX_HAVE_EPOLL)
      epoll_event ev;
      ev.data.fd = s;
      ev.events  = to_events(request);
      throw_errno_if_minus1_("probe::modify()", boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_MOD, s, &ev));
#elif defined(IOXX_HAVE_SELECT)
      if (request & ev_readable) { FD_SET(s, &_read_fds); }   else { FD_CLR(s, &_read_fds); }
      if (request & ev_writable) { FD_SET(s, &_write_fds); }  else { FD_CLR(s, &_write_fds); }
      if (request & ev_pridata)  { FD_SET(s, &_except_fds); } else { FD_CLR(s, &_except_fds); }
#endif
    }

    bool empty() const { return _handlers.empty(); }

    void run(seconds_t timeout)
    {
#if defined(IOXX_HAVE_EPOLL)
      timeout = std::min(timeout, static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000));
      epoll_event ev;
      int const rc( epoll_wait(_epoll_fd, &ev, 1, static_cast<int>(timeout) * 1000) );
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "epoll_wait(2)");
        throw err;
      }
      if (rc == 0) return;
      BOOST_ASSERT(rc == 1);
      iterator const i( _handlers.find(ev.data.fd) );
      BOOST_ASSERT(i != _handlers.end());
      if (i != _handlers.end())
      {
        socket_event sev( ev_idle );
        if (ev.events & EPOLLIN)  sev |= ev_readable;
        if (ev.events & EPOLLOUT) sev |= ev_writable;
        if (ev.events & EPOLLPRI) sev |= ev_pridata;
        i->second(sev);         // this is dangerous in case of suicides
      }
#elif defined(IOXX_HAVE_SELECT)
      timeval tv = { timeout, 0 };
      int rc( select(_handlers.rbegin()->first + 1, &_read_fds, &_write_fds, &_except_fds, &tv) );
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "epoll_wait(2)");
        throw err;
      }
      for (iterator i( _handlers.begin() ); rc != 0; ++i)
      {
        BOOST_ASSERT(rc >= 0);
        BOOST_ASSERT(i != _handlers.end());
        socket_event sev( ev_idle );
        if (FD_ISSET(i->first, &_read_fds))   { --rc; sev |= ev_readable; }
        if (FD_ISSET(i->first, &_write_fds))  { --rc; sev |= ev_writable; }
        if (FD_ISSET(i->first, &_except_fds)) { --rc; sev |= ev_pridata; }
        if (sev != ev_idle)
          i->second(sev);         // this is dangerous in case of suicides
      }
#endif
    }

  private:
    handler_map _handlers;

#if defined(IOXX_HAVE_EPOLL)
    socket_t    _epoll_fd;

    static short to_events(socket_event sev)
    {
      short ev( 0 );
      if (sev & ev_readable) ev |= EPOLLIN;
      if (sev & ev_writable) ev |= EPOLLOUT;
      if (sev & ev_pridata)  ev |= EPOLLPRI;
      return ev;
    }
#elif defined(IOXX_HAVE_SELECT)
    fd_set      _read_fds, _write_fds, _except_fds;
#endif
  };

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED_2008_04_20
