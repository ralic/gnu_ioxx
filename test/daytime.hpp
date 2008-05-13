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

#ifndef IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_04_14
#define IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_04_14

#include <ioxx/core.hpp>
#include <ioxx/acceptor.hpp>
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
  typedef typename socket::handler      handler;
  typedef typename socket::address      address;
  typedef typename socket::native_t     native_socket_t;

  static void accept(io_core & io, native_socket_t s, address const & peer)
  {
    boost::shared_ptr<daytime> p;
    p.reset(new daytime(io, s));
    p->_sock->modify(boost::bind(&daytime::run, p, _1), socket::writable);;
    p->_timeout.in(10u, boost::bind(&daytime::shutdown, p));
  }

private:
  daytime(io_core & io, native_socket_t s) : _sock(new socket(io, s)), _timeout(io)
  {
  }

  void shutdown()
  {
    _timeout.cancel();
    _sock.reset();
  }

  void run(event_set ev)
  {
    try
    {
      BOOST_ASSERT(ev == socket::writable);
      if (!_data_begin)
      {
        tm tstamp;
        localtime_r(&_sock->get_core().as_time_t(), &tstamp);
        size_t const len( strftime( &_buf[0], _buf.size()
                                  , "%d %b %Y %H:%M:%S %Z\r\n"
                                  , &tstamp
                                  ));
        BOOST_ASSERT(len > 0);
        _data_begin = _buf.begin();
        _data_end   = _data_begin + len;
      }

      if (_data_begin != _data_end)
      {
        char const * const p( _sock->write(_data_begin, _data_end) );
        if (!p) return shutdown();
        BOOST_ASSERT(p <= _data_end);
        _data_begin = p;
        _timeout.in(10u, boost::bind(&daytime::shutdown, this->shared_from_this()));
      }

      if (_data_begin == _data_end)
        return shutdown();
    }
    catch(std::exception const & e)
    {
      std::cout << "*** daytime error: " << e.what() << std::endl;
      shutdown();
    }
  }

  socket_ptr                    _sock;
  timeout                       _timeout;

  boost::array<char, 64u>       _buf;
  char const *                  _data_begin;
  char const *                  _data_end;
};

#endif // IOXX_TEST_DAYTIME_HPP_INCLUDED_2008_04_14
