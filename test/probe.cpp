/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#include "ioxx/probe.hpp"
#include "ioxx/timeout.hpp"
#include "ioxx/type/byte.hpp"
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

using namespace std;

/**
 *  \todo It sucks that echo must derive publicly.
 *
 *  \todo Sockets must be made non-blocking in some portable way.
 */
class echo : public ioxx::socket
{
  ioxx::weak_socket const       _sin;
  ioxx::weak_socket const       _sout;
  boost::array<char, 1024>      _buffer;
  ioxx::byte_size               _size;
  ioxx::byte_size               _gap;

public:
  typedef boost::shared_ptr<echo> pointer;

  echo(ioxx::weak_socket inout)
  : _sin(inout), _sout(inout), _size(0u), _gap(0u)
  {
    cerr << "creating full-duplex echo handler " << this << endl;
    BOOST_REQUIRE(inout >= 0);
  }

  echo(ioxx::weak_socket in, ioxx::weak_socket out)
  : _sin(in), _sout(out), _size(0u), _gap(0u)
  {
    cerr << "creating connecting echo handler " << this << endl;
    BOOST_REQUIRE(in >= 0); BOOST_REQUIRE(out >= 0);
  }

  ~echo()
  {
    cerr << "destroy echo handler " << this << endl;
  }

  void shutdown(ioxx::socket::probe & p)
  {
    shutdown(p, ioxx::invalid_weak_socket());
  }

private:
  bool input_blocked(ioxx::weak_socket s) const
  {
    bool const want_read( s == _sin && _size == 0u );
    cerr << "socket " << s << ": requests" << (want_read ? "" : " no") << " input" << endl;
    return want_read;
  }

  bool output_blocked(ioxx::weak_socket s) const
  {
    bool const want_write( s == _sout && _size != 0u );
    cerr << "socket " << s << ": requests" << (want_write ? "" : " no") << " output" << endl;
    return want_write;
  }

  void unblock_input(ioxx::socket::probe & p, ioxx::weak_socket s)
  {
    cerr << "socket " << s << ": is readable" << endl;
    BOOST_REQUIRE_EQUAL(s, _sin);
    BOOST_REQUIRE_EQUAL(_size, 0u);
    int const rc( read(_sin, _buffer.begin(), _buffer.size()) );
    cerr << "socket " << s << ": read " << rc << " bytes" << endl;
    BOOST_CHECK(rc >= 0);
    if (rc <= 0)        echo::shutdown(p, s);
    else
    {
      _size = static_cast<size_t>(rc);
      p.force(_sout);
    }
  }

  void unblock_output(ioxx::socket::probe & p, ioxx::weak_socket s)
  {
    cerr << "socket " << s << ": is writable" << endl;
    if (s == _sin) return;
    BOOST_REQUIRE_EQUAL(s, _sout);
    BOOST_REQUIRE(_size);
    BOOST_REQUIRE(_gap < _size);
    int const rc( write(_sout, _buffer.begin() + _gap, _size) );
    cerr << "socket " << s << ": wrote " << rc << " bytes" << endl;
    BOOST_CHECK(rc > 0);
    BOOST_CHECK(static_cast<size_t>(rc) <= _size);
    if (rc <= 0)        echo::shutdown(p, s);
    else
    {
      _gap  += static_cast<size_t>(rc);
      _size -= static_cast<size_t>(rc);
      if (!_size)
      {
        _gap = 0u;
        p.force(_sin);
      }
    }
  }

  void shutdown(ioxx::socket::probe & p, ioxx::weak_socket)
  {
    cerr << "unregister echo handler " << this << endl;
    p.remove(_sin);
    p.remove(_sout);
  }
};

BOOST_AUTO_TEST_CASE( test_probe )
{
  boost::scoped_ptr<ioxx::socket::probe>  probe(ioxx::socket::probe::make());
  ioxx::timeout                           timer;
  BOOST_REQUIRE(probe);
  {
    echo::pointer p( new echo(STDIN_FILENO, STDOUT_FILENO) );
    probe->insert(STDIN_FILENO, p);
    probe->insert(STDOUT_FILENO, p);
    timer.in(5u, boost::bind(&echo::shutdown, p, boost::ref(*probe)));
  }
  for(;;)
  {
    ioxx::second_t idle_time;
    timer.deliver(&idle_time);
    if (!probe->empty())
      probe->run_once(timer.empty() ? -1 : static_cast<int>(idle_time));
    else
      break;
  }
}
