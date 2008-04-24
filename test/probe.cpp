#include "ioxx/timer.hpp"
#include "ioxx/scheduler.hpp"
#include "ioxx/probe.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

namespace ioxx
{
  inline void nonblocking(socket_t s, bool enable = true)
  {
    IOXX_TRACE_SOCKET(s, (enable ? "enable" : "disable") << " nonblocking mode");
    BOOST_ASSERT(s >= 0);
    int const rc( throw_errno_if_minus1("cannot obtain socket flags", boost::bind<int>(&fcntl, s, F_GETFL, 0)) );
    int const flags( enable ? rc | O_NONBLOCK : rc & ~O_NONBLOCK );
    if (rc != flags)
      throw_errno_if_minus1("cannot set socket flags", boost::bind<int>(&fcntl, s, F_SETFL, flags));
  }

  typedef boost::function<void (socket_t, sockaddr const *, socklen_t)> socket_handler;

  inline void accept_stream_socket(socket_t ls, socket_handler f)
  {
    IOXX_TRACE_SOCKET(ls, "listen socket has become readable");
    sockaddr  addr;
    socklen_t len( sizeof(sockaddr) );
    for (socket_t s( accept(ls, &addr, &len) ); s >= 0; len = sizeof(sockaddr), s = accept(ls, &addr, &len))
    {
      try
      {
        IOXX_TRACE_SOCKET(ls, "accepted new socket " << s);
        nonblocking(s);
        f(s, &addr, len);
      }
      catch(...)
      {
        throw_errno_if_minus1("cannot close() listening socket", boost::bind(&close, s));
        throw;
      }
    }
    if (errno != EWOULDBLOCK && errno != EAGAIN)
    {
      boost::system::system_error err(errno, boost::system::errno_ecat, "accept(2)");
      throw err;
    }
    else
      IOXX_TRACE_SOCKET(ls, "no more pending connections");
  }

  inline socket_t accept_stream_socket(probe<> & p, char const * node, char const * service, socket_handler const & f)
  {
    boost::shared_ptr<addrinfo> _addr;
    addrinfo const hint = { AI_NUMERICHOST, 0, SOCK_STREAM, 0, 0u, NULL, NULL, NULL };
    addrinfo * addr;
    int const rc( getaddrinfo(node, service, &hint, &addr) );
    if (rc != 0) throw std::runtime_error( gai_strerror(rc) );
    _addr.reset(addr, &freeaddrinfo);
    while (addr && addr->ai_socktype != SOCK_STREAM)
      addr = addr->ai_next;
    if (!addr) throw std::runtime_error("address does not map to a stream socket endpoint");
    socket_t const ls( throw_errno_if_minus1( "socket(2)"
                                            , boost::bind(&::socket, addr->ai_family, addr->ai_socktype, addr->ai_protocol)
                                            ));
    try
    {
      throw_errno_if_minus1("bind(2)", boost::bind(&bind, ls, addr->ai_addr, addr->ai_addrlen));
      throw_errno_if_minus1("listen(2)", boost::bind(&listen, ls, 16u));
      nonblocking(ls);
      p.set(ls, boost::bind(&accept_stream_socket, ls, f), ev_readable);
    }
    catch(...)
    {
      throw_errno_if_minus1("cannot close() listening socket", boost::bind(&::close, ls));
      throw;
    }
    return ls;
  }
}

///// Tests ///////////////////////////////////////////////////////////////////

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_socket_event_operators )
{
  using namespace ioxx;

  socket_event ev( ev_readable );
  BOOST_REQUIRE_EQUAL(ev, ev_readable);
  BOOST_REQUIRE(ev & ev_readable);
  BOOST_REQUIRE(!(ev & ev_writable));
  BOOST_REQUIRE(!(ev & ev_pridata));

  ev |= ev_writable;
  BOOST_REQUIRE_EQUAL((int)(ev), (int)(ev_readable) | (int)(ev_writable));
  BOOST_REQUIRE(ev & ev_readable);
  BOOST_REQUIRE(ev & ev_writable);
  BOOST_REQUIRE(!(ev & ev_pridata));

  ev = ev_writable | ev_pridata;
  BOOST_REQUIRE_EQUAL((int)(ev), (int)(ev_pridata) | (int)(ev_writable));
  BOOST_REQUIRE(ev & ev_pridata);
  BOOST_REQUIRE(ev & ev_writable);
  BOOST_REQUIRE(!(ev & ev_readable));
}

BOOST_AUTO_TEST_CASE( test_that_probe_can_be_used_as_sleep )
{
  ioxx::timer           now;
  ioxx::probe<>         probe;
  ioxx::time_t const    startup( now.as_time_t() );
  probe.run( 1u );
  now.update();
  ioxx::time_t const    done( now.as_time_t() );
  int const             runtime( done - startup );
  BOOST_REQUIRE(probe.empty());
  BOOST_REQUIRE_PREDICATE(std::greater_equal<int>(), (runtime)(1));
}

class echo
{
  ioxx::probe<> *               _probe;
  ioxx::socket_t                _sock;
  size_t                        _capacity;
  boost::shared_array<char>     _buffer;
  size_t                        _size;
  size_t                        _gap;

public:
  static void accept(ioxx::probe<> * probe, ioxx::socket_t sock)
  {
    BOOST_REQUIRE(probe);
    probe->set(sock, echo(*probe, sock), ioxx::ev_readable);
  }

  echo(ioxx::probe<> & probe, ioxx::socket_t sock, size_t capacity = 1024u)
  : _probe(&probe), _sock(sock), _capacity(capacity), _buffer(new char[_capacity]), _size(0u), _gap(0u)
  {
    IOXX_TRACE_SOCKET(_sock, "creating echo handler");
    BOOST_REQUIRE(_sock >= 0);
    BOOST_REQUIRE(capacity != 0);
  }

  void operator() (ioxx::socket_event ev)
  {
    IOXX_TRACE_SOCKET(_sock, "socket event " << ev);
    using ioxx::throw_errno_if_minus1;
    using boost::bind;
    try
    {
      if (ev & ioxx::ev_readable)
      {
        BOOST_REQUIRE_EQUAL(_size, 0u);
        ssize_t const rc( throw_errno_if_minus1( "read(2)"
                                               , bind(&read, _sock, &_buffer[0], _capacity)
                                               ));
        IOXX_TRACE_SOCKET(_sock, "read " << rc << " bytes");
        BOOST_REQUIRE(rc >= 0u);
        if (rc == 0) throw std::runtime_error("reached end of input");
        _size = static_cast<size_t>(rc);
        _probe->modify(_sock, ioxx::ev_writable);
      }
      if (ev & ioxx::ev_writable)
      {
        BOOST_REQUIRE(_size);
        BOOST_REQUIRE(_gap + _size <= _capacity);
        ssize_t const rc( throw_errno_if_minus1<ssize_t>( "write(2)"
                                                        , bind(&write, _sock, &_buffer[_gap], _size)
                                                        ));
        IOXX_TRACE_SOCKET(_sock, "wrote " << rc << " bytes");
        BOOST_REQUIRE(rc > 0);
        BOOST_REQUIRE(static_cast<size_t>(rc) <= _size);
        _gap  += static_cast<size_t>(rc);
        _size -= static_cast<size_t>(rc);
        if (_size == 0u)
        {
          _gap = 0u;
          _probe->modify(_sock, ioxx::ev_readable);
        }
      }
    }
    catch(std::exception const & e)
    {
      IOXX_TRACE_SOCKET(_sock, "socket event " << e.what());
      ioxx::socket_t const s(_sock);
      _probe->unset(_sock);     // suicide destroys _sock member
      ioxx::throw_errno_if_minus1("close(2)", boost::bind(&close, s));
    }
  }
};

void close_socket(ioxx::probe<> * p, ioxx::socket_t s)
{
  IOXX_TRACE_SOCKET(s, "shutting down i/o service");
  p->unset(s);
  ioxx::throw_errno_if_minus1("cannot close() socket", boost::bind(&close, s));
}

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  ioxx::timer       now;
  ioxx::scheduler<> schedule;
  ioxx::probe<>     probe;
  ioxx::socket_t const ls( ioxx::accept_stream_socket(probe, "127.0.0.1", "8080", boost::bind(&echo::accept, &probe, _1)) );
  IOXX_TRACE_SOCKET(ls, "accepting connections on port 8080");
  schedule.at(now.as_time_t() + 5, boost::bind(&close_socket, &probe, ls));
  for (;;)
  {
    ioxx::seconds_t timeout( schedule.run(now.as_time_t()) );
    if (schedule.empty())
    {
      if (probe.empty())  break;
      else                timeout = probe.max_timeout();
    }
    probe.run(timeout);
    now.update();
  }
}
