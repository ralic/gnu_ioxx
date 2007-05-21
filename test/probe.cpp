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
using namespace ioxx;

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
class echo : public ioxx::socket
{
  ioxx::probe &                 _probe;
  ioxx::system::socket const    _sin;
  ioxx::system::socket const    _sout;
  boost::array<char, 1024>      _buffer;
  byte_size                     _size;

public:
  echo(ioxx::probe & p, ioxx::system::socket const & inout)
  : _probe(p), _sin(inout), _sout(inout), _size(0u)
  {
    cerr << "creating full-duplex echo handler " << this << endl;
    BOOST_REQUIRE(inout >= 0);
  }

  echo(ioxx::probe & p, ioxx::system::socket const & in, ioxx::system::socket const & out)
  : _probe(p), _sin(in), _sout(out), _size(0u)
  {
    cerr << "creating connecting echo handler " << this << endl;
    BOOST_REQUIRE(in >= 0); BOOST_REQUIRE(out >= 0);
  }

  ~echo()
  {
    cerr << "destroy echo handler " << this << endl;
  }

private:
  bool input_blocked() const    { cerr << "want read: "  << (_size == 0u) << endl; return _size == 0u; }
  bool output_blocked() const   { cerr << "want write: " << (_size != 0u) << endl; return _size != 0u; }

  void unblock_input()
  {
    cerr << "descriptor " << _sin << " is readable" << endl;
    BOOST_REQUIRE_EQUAL(_size, 0u);
    int const rc( read(_sin, _buffer.begin(), _buffer.size()) );
    cerr << "read returned " << rc << endl;
    BOOST_REQUIRE(rc >= 0);
    if (rc == 0)
    {
      _probe.remove(_sin);
      _probe.remove(_sout);
    }
    else
      _size = static_cast<size_t>(rc);
  }

  void unblock_output()
  {
    BOOST_REQUIRE(_size);
    cerr << "descriptor " << _sin << " is writable" << endl;
  }
};

BOOST_AUTO_TEST_CASE( test_probe )
{
  boost::scoped_ptr<probe>  probe(make_probe_poll());
  BOOST_REQUIRE(probe);
  ioxx::socket::pointer p( new echo(*probe, STDIN_FILENO, STDOUT_FILENO) );
  probe->insert(STDIN_FILENO, p);
  probe->insert(STDOUT_FILENO, p);
  while (!probe->empty()) probe->run_once();
}
