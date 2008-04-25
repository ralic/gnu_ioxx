#ifndef IOXX_PROBE_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_HPP_INCLUDED_2008_04_20

#include "socket.hpp"
#include "socket-event.hpp"
#include <boost/function/function1.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <map>

#if defined(IOXX_HAVE_EPOLL)
#  include "probe-epoll-impl.hpp"
namespace ioxx { typedef probe_epoll_implementation default_probe_implementation; }
#elif defined(IOXX_HAVE_POLL)
#  include "probe-poll-impl.hpp"
namespace ioxx { typedef probe_poll_implementation<> default_probe_implementation; }
#elif defined(IOXX_HAVE_SELECT)
#  include "probe-select-impl.hpp"
namespace ioxx { typedef probe_select_implementation default_probe_implementation; }
#else
#  error "No I/O probe implementation available for this platform."
#endif

namespace ioxx
{
  template < class Handler        = boost::function1<void, socket_event>
           , class Allocator      = std::allocator< std::pair<socket_t const, Handler> >
           , class Implementation = default_probe_implementation
           >
  class probe : private boost::noncopyable
  {
    typedef std::map<socket_t,Handler,std::less<socket_t>,Allocator>    handler_map;
    typedef typename handler_map::iterator                              iterator;

  public:
    typedef Handler handler;

    static seconds_t max_timeout() { return Implementation::max_timeout(); }

    explicit probe(unsigned int size_hint = 512u) : _impl(size_hint) { }

    void set(socket_t s, handler f, socket_event request = ev_idle)
    {
      IOXX_TRACE_SOCKET(s, "add to probe for events " << request);
      BOOST_ASSERT(s >= 0);
      std::pair<iterator,bool> const r( _handlers.insert(std::make_pair(s, f)) );
      if (!r.second) std::swap(r.first->second, f);
      try
      {
        if (r.second) _impl.set(s, request);
        else          _impl.modify(s, request);
      }
      catch(...)
      {
        if (r.second) _handlers.erase(r.first);
        else          std::swap(r.first->second, f);
        throw;
      }
    }

    void unset(socket_t s)
    {
      IOXX_TRACE_SOCKET(s, "remove from probe");
      BOOST_ASSERT(s >= 0);
      if (_handlers.erase(s)) _impl.unset(s);
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
      _impl.modify(s, request);
    }

    bool empty() const { return _handlers.empty(); }

    void run(seconds_t timeout)
    {
      timeout = std::min(timeout, max_timeout());
      IOXX_TRACE_MSG("probe " << _handlers.size() << " sockets for i/o, time out after " << timeout << " seconds");
      if (deliver_events()) return;
      _impl.wait(timeout);
      deliver_events();
    }

  private:
    handler_map    _handlers;
    Implementation _impl;

    bool deliver_events()
    {
      bool had_events( false );
      socket_t s;
      socket_event sev;
      while (_impl.pop_event(s, sev))
      {
        IOXX_TRACE_SOCKET(s, "deliver events " << sev);
        BOOST_ASSERT(s >= 0);
        BOOST_ASSERT(sev != ev_idle);
        iterator const i( _handlers.find(s) );
        if (i == _handlers.end()) continue;
        had_events = true;
        i->second(sev);         // this is dangerous in case of suicides
      }
      return had_events;
    }
  };

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED_2008_04_20
