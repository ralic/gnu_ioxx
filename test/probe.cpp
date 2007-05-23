/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
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
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

using namespace std;

/**
 *  \todo It sucks that echo must derive publicly.
 *
 *  \todo Echo must be able to terminate without needing a reference to probe.
 *        One way to achieve that is to make probe de-register a handler that
 *        doesn't probe for anything. Alternatively, calling probe::remove()
 *        can remain the only way to terminate, but then probe should
 *        consistently pass reference back to itself to the callback.
 *
 *  \todo Sockets must be made non-blocking in some portable way.
 */
class echo : public ioxx::probe::socket
{
  ioxx::weak_socket const       _sin;
  ioxx::weak_socket const       _sout;
  boost::array<char, 1024>      _buffer;
  ioxx::byte_size               _size;
  ioxx::byte_size               _gap;

public:
  typedef boost::shared_ptr<echo> pointer;

  echo(ioxx::weak_socket const & inout)
  : _sin(inout), _sout(inout), _size(0u), _gap(0u)
  {
    cerr << "creating full-duplex echo handler " << this << endl;
    BOOST_REQUIRE(inout >= 0);
  }

  echo(ioxx::weak_socket const & in, ioxx::weak_socket const & out)
  : _sin(in), _sout(out), _size(0u), _gap(0u)
  {
    cerr << "creating connecting echo handler " << this << endl;
    BOOST_REQUIRE(in >= 0); BOOST_REQUIRE(out >= 0);
  }

  ~echo()
  {
    cerr << "destroy echo handler " << this << endl;
  }

private:
  bool input_blocked(ioxx::weak_socket const & s) const
  {
    bool const want_read( s == _sin && _size == 0u );
    cerr << "socket " << s << ": requests" << (want_read ? "" : " no") << " input" << endl;
    return want_read;
  }

  bool output_blocked(ioxx::weak_socket const & s) const
  {
    bool const want_write( s == _sout && _size != 0u );
    cerr << "socket " << s << ": requests" << (want_write ? "" : " no") << " output" << endl;
    return want_write;
  }

  void unblock_input(ioxx::probe & p, ioxx::weak_socket const & s)
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

  void unblock_output(ioxx::probe & p, ioxx::weak_socket const & s)
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

  void shutdown(ioxx::probe & p, ioxx::weak_socket const & s)
  {
    cerr << "unregister echo handler " << this << endl;
    p.remove(_sin);
    p.remove(_sout);
  }

};

BOOST_AUTO_TEST_CASE( test_probe )
{
  boost::scoped_ptr<ioxx::probe>  probe(ioxx::make_probe_poll());
  BOOST_REQUIRE(probe);
  {
    echo::pointer p( new echo(STDIN_FILENO, STDOUT_FILENO) );
    probe->insert(STDIN_FILENO, p);
    probe->insert(STDOUT_FILENO, p);
  }
  while (!probe->empty()) probe->run_once();
}
