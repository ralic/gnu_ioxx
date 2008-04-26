#ifndef IOXX_PROBE_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_HPP_INCLUDED_2008_04_20

#include "demux.hpp"
#include <boost/function/function1.hpp>
#include <map>

namespace ioxx
{
  template < class Demuxer   = default_demux
           , class Handler   = boost::function1<void, typename Demuxer::socket::event_set>
           , class Allocator = std::allocator< std::pair<native_socket_t const, Handler> >
           >
  class probe : public Demuxer
  {
  public:
    typedef Demuxer                                                                     demux;
    typedef Handler                                                                     handler;
    typedef typename demux::socket::event_set                                           event_set;

    typedef std::map<native_socket_t,Handler,std::less<native_socket_t>,Allocator>      handler_map;
    typedef typename handler_map::iterator                                              iterator;

    class socket : public demux::socket
    {
    public:
      typedef typename demux::socket::event_set event_set;

      socket(probe & p, native_socket_t sock, handler const & f, event_set ev = demux::socket::no_events)
      : demux::socket(p, sock, ev), _probe(p)
      {
        BOOST_ASSERT(sock >= 0);
        std::pair<iterator,bool> const r( _probe._handlers.insert(std::make_pair(sock, f)) );
        _iter = r.first;
        BOOST_ASSERT(r.second);
      }

      ~socket()
      {
        _probe._handlers.erase(_iter);
      }

      void modify(handler const & f)
      {
        _iter->second = f;
      }

      void modify(handler const & f, event_set ev)
      {
        modify(f);
        demux::request(ev);
      }

    private:
      probe &   _probe;
      iterator  _iter;
    };

    explicit probe(unsigned int size_hint = 512u) : demux(size_hint) { }

    bool empty() const { return _handlers.empty(); }

    void run(seconds_t timeout)
    {
      IOXX_TRACE_MSG("probe " << _handlers.size() << " sockets for i/o, time out after " << timeout << " seconds");
      if (deliver_events()) return;
      demux::wait(timeout);
      deliver_events();
    }

  private:
    handler_map    _handlers;

    bool deliver_events()
    {
      bool had_events( false );
      native_socket_t s;
      event_set ev;
      while (pop_event(s, ev))
      {
        IOXX_TRACE_SOCKET(s, "deliver events " << ev);
        BOOST_ASSERT(s >= 0);
        BOOST_ASSERT(ev != socket::no_events);
        iterator const i( _handlers.find(s) );
        if (i == _handlers.end()) continue;
        had_events = true;
        i->second(ev);         // this is dangerous in case of suicides
      }
      return had_events;
    }
  };

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED_2008_04_20
