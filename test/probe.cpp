#include "ioxx/probe.hpp"
// #include "ioxx/timeout.hpp"
// #include "ioxx/type/byte.hpp"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#if 0
using namespace std;

class echo
{
  ioxx::socket_t const          _sin;
  ioxx::socket_t const          _sout;
  boost::array<char, 1024>      _buffer;
  size_t                        _size;
  size_t                        _gap;

public:
  typedef boost::shared_ptr<echo> pointer;

  echo(ioxx::socket_t inout)
  : _sin(inout), _sout(inout), _size(0u), _gap(0u)
  {
    cerr << "creating full-duplex echo handler " << this << endl;
    BOOST_REQUIRE(inout >= 0);
  }

  echo(ioxx::socket_t in, ioxx::socket_t out)
  : _sin(in), _sout(out), _size(0u), _gap(0u)
  {
    cerr << "creating connecting echo handler " << this << endl;
    BOOST_REQUIRE(in >= 0); BOOST_REQUIRE(out >= 0);
  }

  ~echo()
  {
    cerr << "destroy echo handler " << this << endl;
  }

  void shutdown(ioxx::probe & p)
  {
    shutdown(p, ioxx::invalid_socket_t());
  }

private:
  bool input_blocked(ioxx::socket_t s) const
  {
    bool const want_read( s == _sin && _size == 0u );
    cerr << "socket " << s << ": requests" << (want_read ? "" : " no") << " input" << endl;
    return want_read;
  }

  bool output_blocked(ioxx::socket_t s) const
  {
    bool const want_write( s == _sout && _size != 0u );
    cerr << "socket " << s << ": requests" << (want_write ? "" : " no") << " output" << endl;
    return want_write;
  }

  void unblock_input(ioxx::socket::probe & p, ioxx::socket_t s)
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

  void unblock_output(ioxx::socket::probe & p, ioxx::socket_t s)
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

  void shutdown(ioxx::probe & p, ioxx::socket_t)
  {
    cerr << "unregister echo handler " << this << endl;
    p.remove(_sin);
    p.remove(_sout);
  }
};
#endif

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_probe )
{
  ioxx::probe probe;
#if 0
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
#endif
}
