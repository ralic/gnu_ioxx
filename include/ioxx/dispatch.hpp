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

#include <ioxx/detail/config.hpp>
#if defined(IOXX_HAVE_EPOLL) && IOXX_HAVE_EPOLL
#  include <ioxx/detail/epoll.hpp>
#elif defined(IOXX_HAVE_POLL) && IOXX_HAVE_POLL
#  include <ioxx/detail/poll.hpp>
#elif defined(IOXX_HAVE_SELECT) && IOXX_HAVE_SELECT
#  include <ioxx/detail/select.hpp>
#else
#  error "No I/O de-multiplexer available for this platform."
#endif
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
  template < class Allocator  = std::allocator<void>
           , class Demux      =
#if defined(IOXX_HAVE_EPOLL) && IOXX_HAVE_EPOLL
                                detail::epoll
#elif defined(IOXX_HAVE_POLL) && IOXX_HAVE_POLL
                                detail::poll< typename Allocator::template rebind<pollfd>::other
                                            , typename Allocator::template rebind< std::pair<native_socket_t const, size_t> >::other
                                            >
#elif defined(IOXX_HAVE_SELECT) && IOXX_HAVE_SELECT
                                detail::select
#endif
           , class Handler    = boost::function1< void
                                                , typename Demux::socket::event_set
                                                , typename Allocator::template rebind<boost::function_base>::other
                                                >
           , class HandlerMap = std::map< native_socket_t
                                        , Handler
                                        , std::less<native_socket_t>
                                        , typename Allocator::template rebind< std::pair<native_socket_t const, Handler> >::other
                                        >
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
      typedef typename dispatch::handler        handler;

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

    bool empty() const { return _handlers.empty(); }

    void run()
    {
      native_socket_t s;
      event_set ev;
      while (pop_event(s, ev))
      {
        BOOST_ASSERT(s >= 0);
        BOOST_ASSERT(ev != socket::no_events);
        iterator const i( _handlers.find(s) );
        if (i == _handlers.end())
        {
          LOGXX_MSG_TRACE(this->LOGXX_SCOPE_NAME, "ignore events; handler for socket " << s << " does no longer exist");
          continue;
        }
        i->second(ev);         // this is dangerous in case of suicides
      }
    }

    void wait(seconds_t timeout)
    {
      LOGXX_MSG_TRACE(this->LOGXX_SCOPE_NAME, "probe " << _handlers.size() << " sockets; time out after " << timeout << " seconds");
      demux::wait(timeout);
    }

  private:
    handler_map    _handlers;
  };

} // namespace ioxx

#endif // IOXX_DISPATCH_HPP_INCLUDED_2008_04_20
