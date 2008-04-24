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

  inline std::ostream & operator<< (std::ostream & os, socket_event sev)
  {
    if (sev == ev_idle)    os << "Idle";
    if (sev & ev_readable) os << "Read";
    if (sev & ev_writable) os << "Write";
    if (sev & ev_pridata)  os << "Pridat";
    return os;
  }

  inline std::ostream & operator<< (std::ostream & os, fd_set const & fds)
  {
    for (int i( 0 ); i <= 32 /* FD_SETSIZE */; ++i)
      os << (FD_ISSET(i, &fds) ? '1' : '0');
    return os;
  }
  class probe : private boost::noncopyable
  {
  public:
    typedef boost::function<void (socket_event)>        handler;
    typedef std::map<socket_t,handler>                  handler_map;
    typedef handler_map::iterator                       iterator;

    static seconds_t max_timeout()
    {
      return
#if defined(IOXX_HAVE_EPOLL)
        static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000)
#elif defined(IOXX_HAVE_SELECT)
        static_cast<seconds_t>(std::numeric_limits<int>::max())
#endif
        ;
    }

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
      IOXX_TRACE_SOCKET(s, "add to probe for events " << request);
      BOOST_ASSERT(s >= 0);
      std::pair<iterator,bool> const r( _handlers.insert(std::make_pair(s, f)) );
      if (!r.second) std::swap(r.first->second, f);
#if defined(IOXX_HAVE_EPOLL)
      try
      {
        epoll_event ev( make_event(s, request) );
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
      reset_events(s, request);
#endif
    }

    void unset(socket_t s)
    {
      IOXX_TRACE_SOCKET(s, "remove from probe");
      BOOST_ASSERT(s >= 0);
      if (!_handlers.erase(s)) return;
#if defined(IOXX_HAVE_EPOLL)
      epoll_event ev( make_event(s, ev_idle) );
      throw_errno_if_minus1_("probe::unset()", boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_DEL, s, &ev));
#elif defined(IOXX_HAVE_SELECT)
      reset_events(s, ev_idle);
#endif
    }

    void modify(socket_t s, handler const & f)
    {
      IOXX_TRACE_SOCKET(s, "modify handler");
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
      _handlers[s] = f;
    }

    void modify(socket_t s, socket_event request)
    {
      IOXX_TRACE_SOCKET(s, "modify requested events to " << request);
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(_handlers.find(s) != _handlers.end());
#if defined(IOXX_HAVE_EPOLL)
      epoll_event ev( make_event(s, request) );
      throw_errno_if_minus1_("probe::modify()", boost::bind(&epoll_ctl, _epoll_fd, EPOLL_CTL_MOD, s, &ev));
#elif defined(IOXX_HAVE_SELECT)
      reset_events(s, request);
#endif
    }

    bool empty() const { return _handlers.empty(); }

    void run(seconds_t timeout)
    {
      timeout = std::min(timeout, max_timeout());
      IOXX_TRACE_MSG("probe " << _handlers.size() << " sockets for i/o, time out after " << timeout << " seconds");
#if defined(IOXX_HAVE_EPOLL)
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
      if (_handlers.empty())
      {
        select(0, NULL, NULL, NULL, &tv);
        return;
      }
      socket_t const nfds( _handlers.rbegin()->first + 1 );
      fd_set read_fds(_read_fds), write_fds(_write_fds), except_fds(_except_fds);
      IOXX_TRACE_MSG("nfds       = " << nfds);
      IOXX_TRACE_MSG("read_fds   = " << _read_fds);
      IOXX_TRACE_MSG("write_fds  = " << _write_fds);
      IOXX_TRACE_MSG("except_fds = " << _except_fds);
      BOOST_ASSERT(nfds > 0);
      int rc( select(nfds, &read_fds, &write_fds, &except_fds, &tv) );
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "select(2)");
        throw err;
      }
      IOXX_TRACE_MSG("nfds       = " << rc);
      IOXX_TRACE_MSG("read_fds   = " << read_fds);
      IOXX_TRACE_MSG("write_fds  = " << write_fds);
      IOXX_TRACE_MSG("except_fds = " << except_fds);
      for (socket_t s( 0 ); s != nfds && rc != 0; ++s)
      {
        BOOST_ASSERT(rc >= 0);
        iterator const i( _handlers.find(s) );
        if (i == _handlers.end()) continue;
        socket_event sev( ev_idle );
        if (FD_ISSET(s, &read_fds))   { --rc; sev |= ev_readable; }
        if (FD_ISSET(s, &write_fds))  { --rc; sev |= ev_writable; }
        if (FD_ISSET(s, &except_fds)) { --rc; sev |= ev_pridata; }
        if (sev != ev_idle)
          i->second(sev);         // this is dangerous in case of suicides
      }
#endif
    }

  private:
    handler_map _handlers;

#if defined(IOXX_HAVE_EPOLL)
    socket_t    _epoll_fd;

    static epoll_event make_event(socket_t s, socket_event sev)
    {
      BOOST_ASSERT(s >= 0);
      epoll_event ev;
      ev.data.fd = s;
      if (sev & ev_readable) ev.events |= EPOLLIN;
      if (sev & ev_writable) ev.events |= EPOLLOUT;
      if (sev & ev_pridata)  ev.events |= EPOLLPRI;
      return ev;
    }
#elif defined(IOXX_HAVE_SELECT)
    fd_set      _read_fds, _write_fds, _except_fds;

    void reset_events(socket_t s, socket_event sev)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(s <= FD_SETSIZE);
      if (sev & ev_readable) { FD_SET(s, &_read_fds); }   else { FD_CLR(s, &_read_fds); }
      if (sev & ev_writable) { FD_SET(s, &_write_fds); }  else { FD_CLR(s, &_write_fds); }
      if (sev & ev_pridata)  { FD_SET(s, &_except_fds); } else { FD_CLR(s, &_except_fds); }
    }
#endif
  };

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED_2008_04_20
