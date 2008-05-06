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
#include <ioxx/acceptor.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

class echo
{
  typedef ioxx::schedule<>              schedule;
  typedef schedule::timeout             timeout;
  typedef ioxx::dispatch<>              dispatch;
  typedef dispatch::socket              socket;
  typedef boost::scoped_ptr<socket>     socket_ptr;
  typedef socket::event_set             event_set;

  socket_ptr                    _sock;
  timeout                       _tout;
  boost::array<char,1024>       _buf;
  size_t                        _len;
  size_t                        _gap;

  echo(schedule & sched) : _tout(sched), _len(0u), _gap(0u)
  {
    BOOST_REQUIRE(_buf.size());
  }

  void run(event_set ev)
  {
    try
    {
      IOXX_TRACE_SOCKET(*_sock, "socket event " << ev);
      if (ev & socket::readable)
      {
        BOOST_REQUIRE_EQUAL(_len, 0u);
        char * const data_end( _sock->read(_buf.begin(), _buf.end()) );
        if (!data_end) throw std::runtime_error("reached end of input");
        BOOST_ASSERT(_buf.begin() < data_end);
        _len = static_cast<size_t>(data_end - _buf.begin());
        _sock->request(socket::writable);
      }
      if (ev & socket::writable)
      {
        BOOST_REQUIRE(_len);
        BOOST_REQUIRE(_gap + _len <= _buf.size());
        char const * const new_begin( _sock->write(&_buf[_gap], &_buf[_gap + _len]) );
        BOOST_REQUIRE(new_begin);
        BOOST_REQUIRE(_buf.begin() < new_begin);
        size_t const n(new_begin - _buf.begin());
        _gap  += n;
        _len -= n;
        if (_len == 0u)
        {
          _gap = 0u;
          _sock->request(socket::readable);
        }
      }
#if 0
      _tout.in(5, boost::bind(&socket::modify, boost::ref(_sock), dispatch::handler()));
#else
      _tout.in(5, boost::bind(&socket_ptr::reset, boost::ref(_sock), static_cast<socket*>(0)));
#endif
    }
    catch(std::exception const & e)
    {
      IOXX_TRACE_SOCKET(*_sock, "socket event: " << e.what());
      _sock.reset();
    }
  }

public:
  ~echo() { IOXX_TRACE_MSG("destroy echo handler"); }

  static void accept(schedule & sched, dispatch & disp, ioxx::native_socket_t s, socket::address const & addr)
  {
    IOXX_TRACE_SOCKET(s, "start echo handler for peer " << addr.show());
    boost::shared_ptr<echo> f;
    f.reset( new echo(sched) );
    socket * sock( new socket(disp, s, boost::bind(&echo::run, f, _1), socket::readable) );
    f->_sock.reset(sock);
    f->run(socket::no_events);
  }
};

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  typedef ioxx::time                    time;
  typedef ioxx::schedule<>              schedule;
  typedef ioxx::dispatch<>              dispatch;
  typedef dispatch::socket              socket;
  typedef socket::address               address;
  typedef socket::endpoint              endpoint;
  typedef ioxx::acceptor<dispatch>      acceptor;
  typedef boost::scoped_ptr<acceptor>   acceptor_ptr;

  using boost::bind;
  using boost::ref;

  time          now;
  schedule      sched(now.as_time_t());
  dispatch      disp;
  acceptor_ptr  ls;
  ls.reset( new acceptor( disp, endpoint("127.0.0.1", "8080")
                        , bind(&echo::accept, ref(sched), ref(disp), _1, _2)
                        ));
  IOXX_TRACE_SOCKET(*ls, "accepting connections on port 8080");
  sched.in(5u, bind(&acceptor_ptr::reset, ref(ls), static_cast<acceptor*>(0)));
  for (;;)
  {
    disp.run();
    ioxx::seconds_t timeout( sched.run() );
    if (sched.empty())
    {
      if (disp.empty()) break;
      else              timeout = disp.max_timeout();
    }
    disp.wait(timeout);
    now.update();
  }
  IOXX_TRACE_MSG("shutting down");
}
