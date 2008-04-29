#ifndef IOXX_DISPATCH_HPP_INCLUDED_2008_04_20
#define IOXX_DISPATCH_HPP_INCLUDED_2008_04_20

#include "demux.hpp"
#include <boost/function/function1.hpp>
#include <map>

namespace ioxx
{
  typedef unsigned int seconds_t;

  template < class Demuxer   = default_demux
           , class Handler   = boost::function1<void, typename Demuxer::socket::event_set>
           , class Allocator = std::allocator< std::pair<native_socket_t const, Handler> >
           >
  class dispatch : protected Demuxer
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

      socket(dispatch & p, native_socket_t sock, handler const & f, event_set ev = demux::socket::no_events)
      : demux::socket(p, sock, ev)
      {
        BOOST_ASSERT(sock >= 0);
        std::pair<iterator,bool> const r( context()._handlers.insert(std::make_pair(sock, f)) );
        _iter = r.first;
        BOOST_ASSERT(r.second);
      }

      ~socket()
      {
        context()._handlers.erase(_iter);
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

    protected:
      dispatch & context() { return static_cast<dispatch &>(demux::socket::context()); }

    private:
      iterator  _iter;
    };

    static seconds_t max_timeout() { return demux::max_timeout(); }

    explicit dispatch(unsigned int size_hint = 512u) : demux(size_hint) { }

    bool empty() const { return _handlers.empty(); }

    void run()
    {
      native_socket_t s;
      event_set ev;
      while (pop_event(s, ev))
      {
        IOXX_TRACE_SOCKET(s, "deliver events " << ev);
        BOOST_ASSERT(s >= 0);
        BOOST_ASSERT(ev != socket::no_events);
        iterator const i( _handlers.find(s) );
        if (i == _handlers.end())
        {
          IOXX_TRACE_SOCKET(s, "ignore events; handler for this socket does no longer exist");
          continue;
        }
        i->second(ev);         // this is dangerous in case of suicides
      }
    }

    void wait(seconds_t timeout) { demux::wait(timeout); }

  private:
    handler_map    _handlers;
  };

} // namespace ioxx

#endif // IOXX_DISPATCH_HPP_INCLUDED_2008_04_20
