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

#include <ioxx/time.hpp>
#include <ioxx/schedule.hpp>
#include <ioxx/dispatch.hpp>
#include <ioxx/dns.hpp>

namespace ioxx
{
  template < class Allocator = std::allocator<void> >
  class io_core
  {
  public:
    typedef Allocator                                                                                   allocator;
    typedef typename Allocator::template rebind<boost::function_base>::other                            function_allocator;

    typedef boost::function0<void, function_allocator>                                                  task;
    typedef typename Allocator::template rebind< std::pair<time_t const, task> >::other                 schedule_allocator;
    typedef schedule<task, schedule_allocator>                                                          schedule;
    typedef typename schedule::task_id                                                                  task_id;
    typedef typename schedule::timeout                                                                  timeout;

    typedef boost::function1<void, demux::socket::event_set, function_allocator>                        handler;
    typedef typename Allocator::template rebind< std::pair<native_socket_t const, handler> >::other     dispatch_allocator;
    typedef std::map<native_socket_t,handler,std::less<native_socket_t>,dispatch_allocator>             dispatch_map;
    typedef dispatch<demux, handler, dispatch_map>                                                      dispatch;
    typedef typename dispatch::event_set                                                                event_set;
    typedef typename dispatch::socket                                                                   socket;

    time &      get_time()      { return _now; }
    schedule &  get_schedule()  { return _schedule; }
    dispatch &  get_dispatch()  { return _dispatch; }

    seconds_t run()
    {
      _dispatch.run();
      seconds_t timeout( _schedule.run(_now.as_time_t()) );
      if (_schedule.empty())
      {
        if (_dispatch.empty())  BOOST_ASSERT(timeout == 0u);
        else                    timeout = _dispatch.max_timeout();
      }
      return timeout;
    }

    void wait(seconds_t timeout)
    {
      _dispatch.wait(timeout);
      _now.update();
    }

  protected:
    time        _now;
    schedule    _schedule;
    dispatch    _dispatch;
  };

} // namespace ioxx

///// test cases //////////////////////////////////////////////////////////////

#include <ioxx/acceptor.hpp>
#include <boost/enable_shared_from_this.hpp>
// #include <boost/scoped_ptr.hpp>
// #include <boost/array.hpp>

template <class IOCore>
class httpd : public boost::enable_shared_from_this< httpd<IOCore> >
{
public:
  typedef IOCore                        io_core;
  typedef httpd<io_core>                self;
  typedef boost::shared_ptr<self>       self_ptr;
  typedef typename io_core::timeout     timeout;
  typedef typename io_core::event_set   event_set;
  typedef typename io_core::socket      socket;
  typedef typename io_core::handler     handler;
  typedef typename socket::address      address;
  typedef typename socket::native_t     native_socket_t;

  static void accept(io_core & io, native_socket_t s, address const & peer)
  {
    self_ptr p;
    p.reset(new httpd(io, s));
    p->_sock.modify(boost::bind(&self::run, p, _1), socket::readable);;
    p->_timeout.reset(io.get_time().as_time_t() + 30u, boost::bind(&socket::modify, boost::ref(p->_sock), handler()));
  }

private:
  httpd(io_core & io, native_socket_t s) : _sock(io.get_dispatch(), s), _timeout(io.get_schedule())
  {
  }

  void run(event_set ev)
  {
  }

  socket        _sock;
  timeout       _timeout;
};

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <boost/pool/pool_alloc.hpp>

static sig_atomic_t stop_service = false;
static void stop_service_hook(int) { stop_service = true; }

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  typedef ioxx::native_socket_t                         native_socket_t;
  typedef boost::fast_pool_allocator<native_socket_t>   allocator;
  typedef ioxx::io_core<allocator>                      io_core;
  typedef io_core::dispatch                             dispatch;
  typedef io_core::socket                               socket;
  typedef socket::address                               address;
  typedef socket::endpoint                              endpoint;
  typedef ioxx::acceptor<dispatch>                      acceptor;

  using boost::bind;
  using boost::ref;

  ioxx::detail::block_signals no_signal_scope;

  io_core io;
  acceptor listen_port(io.get_dispatch(), endpoint("127.0.0.1", "8080"), bind(&httpd<io_core>::accept, ref(io), _1, _2) );
  for (int i(0); i != 32; ++i) ::signal(i, SIG_IGN);
  ioxx::detail::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGINT, &stop_service_hook));
  ioxx::detail::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGTERM, &stop_service_hook));
  while (!stop_service)
  {
    ioxx::seconds_t timeout( io.run() );
    if (!timeout) break;
    io.wait(timeout);
  }
  IOXX_TRACE_MSG("shutting down");
}
