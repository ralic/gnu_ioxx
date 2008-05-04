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

#include "core.hpp"
#include <ioxx/acceptor.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

class echo : public boost::enable_shared_from_this<echo>
{
  ioxx::core::socket            _sock;
  boost::array<char,1024>       _buf;
  size_t                        _len;
  size_t                        _gap;

  echo(ioxx::core & io, ioxx::native_socket_t s) : _sock(io, s), _len(0u), _gap(0u)
  {
    BOOST_REQUIRE(_buf.size());
  }

  void input()
  {
    BOOST_REQUIRE_EQUAL(_len, 0u);
    char * const data_end( _sock.read(_buf.begin(), _buf.end()) );
    if (!data_end) return;
    BOOST_ASSERT(_buf.begin() < data_end);
    _len = static_cast<size_t>(data_end - _buf.begin());
    _sock.on_output(boost::bind(&echo::output, shared_from_this()), 5u, boost::bind(&echo::reset, shared_from_this()));
  }

  void output()
  {
    BOOST_REQUIRE(_len);
    BOOST_REQUIRE(_gap + _len <= _buf.size());
    char const * const new_begin( _sock.write(&_buf[_gap], &_buf[_gap + _len]) );
    BOOST_REQUIRE(new_begin);
    BOOST_REQUIRE(_buf.begin() < new_begin);
    size_t const n(new_begin - _buf.begin());
    _gap  += n;
    _len -= n;
    if (_len == 0u)
    {
      _gap = 0u;
      _sock.on_input(boost::bind(&echo::input, shared_from_this()), 5u, boost::bind(&echo::reset, shared_from_this()));
    }
  }

  void reset() { _sock.reset(); }

public:
  ~echo() { IOXX_TRACE_MSG("destroy echo handler"); }

  static void accept(ioxx::core & io, ioxx::native_socket_t s, ioxx::core::socket::address const & addr)
  {
    IOXX_TRACE_SOCKET(s, "start echo handler for peer " << addr.show());
    boost::shared_ptr<echo> ptr;
    ptr.reset( new echo(io, s) );
    ptr->_sock.on_input(boost::bind(&echo::input, ptr));
  }
};

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  typedef ioxx::core::dispatch          dispatch;
  typedef ioxx::core::socket            socket;
  typedef socket::address               address;
  typedef socket::endpoint              endpoint;
  typedef ioxx::acceptor<dispatch>      acceptor;
  typedef boost::scoped_ptr<acceptor>   acceptor_ptr;

  using boost::bind;
  using boost::ref;

  ioxx::core    io;
  dispatch      disp;
  acceptor_ptr  ls;
  ls.reset( new acceptor( io, endpoint("127.0.0.1", "8080")
                        , bind(&echo::accept, ref(io), _1, _2)
                        ));
  IOXX_TRACE_SOCKET(*ls, "accepting connections on port 8080");
  io.in(5u, bind(&acceptor_ptr::reset, ref(ls), static_cast<acceptor*>(0)));
  io.run();
  IOXX_TRACE_MSG("shutting down");
}
