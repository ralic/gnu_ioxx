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

#include "ioxx/time.hpp"
#include "ioxx/schedule.hpp"
#include "ioxx/dispatch.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ioxx
{
  typedef boost::function<void (native_socket_t, sockaddr const *, socklen_t)> socket_handler;

  inline void accept_socket(native_socket_t ls, socket_handler f)
  {
    using namespace ioxx::detail;
    IOXX_TRACE_SOCKET(ls, "listen socket has become readable");
    native_socket_t     s;
    sockaddr            addr;
    socklen_t           len;
    for (;;)
    {
      len = sizeof(sockaddr);
      s   = throw_errno_if(not_ewould_block(), "accept(2)", boost::bind(&::accept,ls, &addr, &len));
      if (s < 0) break;
      socket sock(s);           // acts as scope guard in case of exceptions
      sock.set_nonblocking();
      f(s, &addr, len);
      sock.close_on_destruction(false);
    }
    IOXX_TRACE_SOCKET(ls, "no more pending connections");
  }

  template <class Demuxer, class Handler, class Allocator>
  inline
  typename dispatch<Demuxer,Handler,Allocator>::socket *
  accept_stream_socket( dispatch<Demuxer,Handler,Allocator> & p
                      , char const * node, char const * service
                      , socket_handler const & f
                      )
  {
    using namespace detail;
    typedef dispatch<Demuxer,Handler,Allocator> dispatch;
    typedef typename dispatch::socket           dispatch_socket;
    boost::shared_ptr<addrinfo> _addr;
    addrinfo const hint = { AI_NUMERICHOST, 0, SOCK_STREAM, 0, 0u, NULL, NULL, NULL };
    addrinfo * addr;
    int const rc( getaddrinfo(node, service, &hint, &addr) );
    if (rc != 0) throw std::runtime_error( gai_strerror(rc) );
    _addr.reset(addr, &freeaddrinfo);
    while (addr && addr->ai_socktype != SOCK_STREAM)
      addr = addr->ai_next;
    if (!addr) throw std::runtime_error("address does not map to a stream socket endpoint");
    socket ls( throw_errno_if_minus1( "socket(2)"
                                    , boost::bind(&::socket, addr->ai_family, addr->ai_socktype, addr->ai_protocol)
                                    ));
    throw_errno_if_minus1("bind(2)", boost::bind(&bind, ls.as_native_socket_t(), addr->ai_addr, addr->ai_addrlen));
    throw_errno_if_minus1("listen(2)", boost::bind(&listen, ls.as_native_socket_t(), 16u));
    ls.set_nonblocking();
    dispatch_socket * ps( new dispatch_socket(p, ls.as_native_socket_t(), boost::bind(&accept_socket, ls.as_native_socket_t(), f), dispatch_socket::readable) );
    ls.close_on_destruction(false);
    return ps;
  }
}

///// Tests ///////////////////////////////////////////////////////////////////

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
  ioxx::time_t const &          _now;

  echo(schedule & sched, ioxx::time_t const & now) : _tout(sched), _len(0u), _gap(0u), _now(now)
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
      _tout.reset(_now + 5, boost::bind(&socket_ptr::reset, boost::ref(_sock), static_cast<socket*>(0)));
    }
    catch(std::exception const & e)
    {
      IOXX_TRACE_SOCKET(*_sock, "socket event: " << e.what());
      _sock.reset();
    }
  }

public:
  ~echo() { IOXX_TRACE_MSG("destroy echo handler"); }

  static void accept(schedule & sched, ioxx::time_t const & now, dispatch & disp, ioxx::native_socket_t s)
  {
    boost::shared_ptr<echo> f;
    f.reset( new echo(sched, now) );
    socket * sock( new socket(disp, s, boost::bind(&echo::run, f, _1), socket::readable) );
    f->_sock.reset(sock);
  }
};

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  ioxx::time        now;
  ioxx::schedule<>  schedule;
  ioxx::dispatch<>  dispatch;
  boost::scoped_ptr<ioxx::dispatch<>::socket> ls;
  ls.reset(ioxx::accept_stream_socket(dispatch, "127.0.0.1", "8080", boost::bind(&echo::accept, boost::ref(schedule), boost::ref(now.as_time_t()), boost::ref(dispatch), _1)));
  IOXX_TRACE_SOCKET(ls, "accepting connections on port 8080");
  schedule.at(now.as_time_t() + 5, boost::bind(&boost::scoped_ptr<ioxx::dispatch<>::socket> ::reset, &ls, static_cast<ioxx::dispatch<>::socket *>(0)));
  for (;;)
  {
    dispatch.run();
    ioxx::seconds_t timeout( schedule.run(now.as_time_t()) );
    if (schedule.empty())
    {
      if (dispatch.empty())  break;
      else                   timeout = dispatch.max_timeout();
    }
    dispatch.wait(timeout);
    now.update();
  }
}
