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

#ifndef IOXX_DISPATCH_HPP_INCLUDED_2008_04_20
#define IOXX_DISPATCH_HPP_INCLUDED_2008_04_20

#include <ioxx/demux.hpp>
#include <boost/function/function1.hpp>
#include <map>

namespace ioxx
{
  /**
   * Unsigned type for representing seconds.
   */
  typedef unsigned int seconds_t;

  /**
   * A simple time-event dispatcher.
   */
  template < class Demux      = demux
           , class Handler    = boost::function1<void, typename Demux::socket::event_set>
           , class HandlerMap = std::map<native_socket_t,Handler>
           >
  class dispatch : protected Demux
  {
  public:
    typedef Demux                                                                       demux;
    typedef Handler                                                                     handler;
    typedef HandlerMap                                                                  handler_map;
    typedef typename handler_map::iterator                                              iterator;
    typedef typename demux::socket::event_set                                           event_set;

    class socket : public demux::socket
    {
    public:
      typedef typename demux::socket::event_set event_set;

      socket(dispatch & disp, native_socket_t sock, handler const & f = handler(), event_set ev = demux::socket::no_events)
      : demux::socket(disp, sock, ev)
      {
        BOOST_ASSERT(this->as_native_socket_t() >= 0);
        std::pair<iterator,bool> const r( context()._handlers.insert(std::make_pair(this->as_native_socket_t(), f)) );
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

      void modify(handler f, event_set ev)
      {
        this->request(ev);
        std::swap(_iter->second, f);
      }

    protected:
      dispatch & context() { return static_cast<dispatch &>(demux::socket::context()); }

    private:
      iterator  _iter;
    };

    static seconds_t max_timeout() { return demux::max_timeout(); }

    dispatch() { }

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
