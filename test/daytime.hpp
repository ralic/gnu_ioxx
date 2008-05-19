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

#ifndef IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_05_19
#define IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_05_19

#include <ioxx/core.hpp>
#include <ioxx/iovec.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>

template <class IOCore>
class daytime : public boost::enable_shared_from_this< daytime<IOCore> >
{
public:
  typedef IOCore                        io_core;
  typedef typename io_core::timeout     timeout;
  typedef typename io_core::event_set   event_set;
  typedef typename io_core::socket      socket;
  typedef boost::scoped_ptr<socket>     socket_ptr;
  typedef typename socket::address      address;
  typedef typename socket::native_t     native_socket_t;

  static void accept(io_core & io, native_socket_t s, address const & peer)
  {
    boost::shared_ptr<daytime> p;
    p.reset(new daytime(io, s, peer));
    io.query_ptr(peer, boost::bind(&daytime::start, p, _1));
  }

private:
  daytime(io_core & io, native_socket_t s, address const & addr)
  : _sock(new socket(io, s)), _timeout(io), _peer(addr)
  {
    LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.daytime(" + ioxx::detail::show(s) + ')');
    LOGXX_TRACE("received daytime request from " << addr);
    tm tstamp;
    localtime_r(&io.as_time_t(), &tstamp);
    size_t const len( strftime( &_buf[0], _buf.size()
                              , "%d %b %Y %H:%M:%S %Z\r\n"
                              , &tstamp
                              ));
    BOOST_ASSERT(len > 0);
    _data_begin = _buf.begin();
    _data_end   = _data_begin + len;
  }

  void start(typename io_core::hostname * peer)
  {
    LOGXX_INFO("peer " << _peer << " resolves to \"" << (peer ? *peer : "NONE") << '"');
    _sock->modify(boost::bind(&daytime::run, this->shared_from_this(), _1), socket::writable);;
    _timeout.in(10u, boost::bind(&daytime::shutdown, this->shared_from_this()));
  }

  void run(event_set ev)
  {
    try
    {
      BOOST_ASSERT(ev == socket::writable);
      BOOST_ASSERT(_data_begin != _data_end);
      ioxx::iovec iov( ioxx::make_iovec(_data_begin, _data_end) );
      ssize_t const rc( _sock->send_to(&iov, &iov + 1, _peer) );
      if (rc < 0) return shutdown(); // connection reset by peer
      _data_begin += rc;
      BOOST_ASSERT(_data_begin <= _data_end);
      if (_data_begin == _data_end) return shutdown();
      _timeout.in(10u, boost::bind(&daytime::shutdown, this->shared_from_this()));
    }
    catch(std::exception const & e)
    {
      LOGXX_ERROR(e.what());
      shutdown();
    }
  }

  void shutdown()
  {
    LOGXX_TRACE("shut down");
    _timeout.cancel();
    _sock.reset();
  }

  socket_ptr                    _sock;
  timeout                       _timeout;
  address                       _peer;

  boost::array<char, 64u>       _buf;
  char const *                  _data_begin;
  char const *                  _data_end;

  LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);
};

#endif // IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_05_19
