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
// #include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include "logging.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

using namespace std;
using namespace ioxx;

#if 0
struct echo : private boost::noncopyable
{
  struct acceptor
  {
    void operator() (socket const & s, probe & p) const
    {
      I(s);
      add_shared_handler(p, s, Readable | Writable, boost::shared_ptr<echo>(new echo(s)));
      throw false;                // shut down acceptor after one connection
    }
  };

  boost::array<char, 1024>      _buffer_array;
  ioxx::stream_buffer<char>     _buf;

  data_socket           _insock, _outsock;
  bool                  _done;

  echo(data_socket const & inout) : _buf(_buffer_array), _insock(inout), _outsock(inout), _done(false)
  {
    IOXX_DEBUG_MSG(catchall) << "create full-duplex echo handler " << this;
    I(inout);
    _insock.nonblocking();
  }

  echo(data_socket in, data_socket out) : _buf(_buffer_array), _insock(in), _outsock(out), _done(false)
  {
    IOXX_DEBUG_MSG(catchall) << "create connecting echo handler " << this;
    I(in); I(out);
    _insock.nonblocking();
    _outsock.nonblocking();
  }

  ~echo()
  {
    IOXX_DEBUG_MSG(catchall) << "destroy echo handler " << this;
    if (_insock == _outsock) _outsock.release();
  }

  void mark_eof()         { _done = true; }
  void invalidate()       { mark_eof(); _buf.reset(); }
  bool can_read()  const  { return !_done && !_buf.full(); }
  bool can_write() const  { return !_buf.empty(); }

  event operator() (socket s, event e, probe & p)
  {
    I(s == _insock || s == _outsock);

    if (is_error(e)) { invalidate(); p.del(s); }

    bool const was_reading = can_read();
    bool const was_writing = can_write();

    try
    {
      /// \todo get in shape for edge-triggered
      if (is_readable(e) && can_read())
      {
        if (!_buf.read(_insock))         mark_eof();
        if (!_buf.full())               e &= ~Readable;
      }

      if (is_writable(e) && can_write())
      {
        _buf.write(_outsock);
        if (!_buf.empty())              e &= ~Writable;
      }
    }
    catch(exception const & ex)
    {
      IOXX_WARN_MSG(catchall) << "caught: " << ex.what();
      invalidate();
    }

    IOXX_DEBUG_MSG(catchall)
      << "was_reading = " << (was_reading ? "yes" : "no") << "; "
      << "was_writing = " << (was_writing ? "yes" : "no") << "; "
      << "can_read = "    << (can_read()  ? "yes" : "no") << "; "
      << "can_write = "   << (can_write() ? "yes" : "no") << "; "
      ;

    event mask = None;

    if (can_read())
    {
      if (s == _insock)         { mask |= Readable; }
      else                      { if (!was_reading) p.modify(_insock, Readable); }
    }

    if (can_write())
    {
      if (s == _outsock)        { mask |= Writable; }
      else                      { if (!was_writing) p.modify(_outsock, Writable); }
    }

    if (can_read() || can_write())      return mask;
    else
    {
      if (_insock  && s != _insock)  p.del(_insock);
      if (_outsock && s != _outsock) p.del(_outsock);
      p.del(s);
      return Error;
    }
  }
};
#endif

BOOST_AUTO_TEST_CASE( test_probe )
{
  boost::scoped_ptr<probe>  probe(make_probe_poll());
  BOOST_ASSERT(probe);

#if 0
  socket sin;
  if (argc > 1)
  {
    IOXX_DEBUG_MSG(catchall) << "filter mode";
    sin = socket(STDIN_FILENO);
  }
  else
  {
    IOXX_DEBUG_MSG(catchall) << "echo /etc/modules.conf";
    sin = socket(open("/etc/modules.conf", O_RDONLY));
  }
  socket sout = socket(STDOUT_FILENO);
  echo eh(sin, sout);
  add_shared_handler(*probe, sin,  Readable, &eh);
  add_shared_handler(*probe, sout, None, &eh);
#endif

  while (!probe->empty()) probe->run_once();
}
