/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_TEST_ECHO_HPP_INCLUDED_2010_02_23
#define IOXX_TEST_ECHO_HPP_INCLUDED_2010_02_23

#include <ioxx/core.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>

template <class IOCore>
class echo : public boost::enable_shared_from_this< echo<IOCore> >
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
    boost::shared_ptr<echo> p;
    p.reset(new echo(io, s, peer));
    io.query_ptr(peer, boost::bind(&echo::start, p, _1));
  }

private:
  echo(io_core & io, native_socket_t s, address const & addr)
  : _sock(new socket(io, s)), _timeout(io), _peer(addr), _data_begin(&_buf[0]), _data_end(&_buf[0])
  {
    LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.echo(" + ioxx::detail::show(s) + ')');
    LOGXX_TRACE("received echo request from " << addr);
  }

  void start(typename io_core::hostname * peer)
  {
    LOGXX_INFO("peer " << _peer << " resolves to \"" << (peer ? *peer : "NONE") << '"');
    _sock->modify(boost::bind(&echo::run, this->shared_from_this(), _1), socket::readable);;
    _timeout.in(10u, boost::bind(&echo::shutdown, this->shared_from_this()));
  }

  void run(event_set ev)
  {
    try
    {
      if (ev & socket::readable)
      {
        BOOST_ASSERT(_data_begin == _data_end);
        _data_begin = _buf.begin();
        _data_end = _sock->read(_buf.begin(), _buf.end());
        if (!_data_end) return shutdown(); // end of input
        BOOST_ASSERT(_data_begin <= _data_end);
        if (_data_begin != _data_end)
          _sock->request(socket::writable);
      }
      if (ev & socket::writable)
      {
        BOOST_ASSERT(_data_begin < _data_end);
        _data_begin = _sock->write(_data_begin, _data_end);
        if (!_data_begin) return shutdown(); // reset by peer
        BOOST_ASSERT(_data_begin <= _data_end);
        if (_data_begin == _data_end)
        {
          _data_begin = _data_end = _buf.begin();
          _sock->request(socket::readable);
        }
      }
      _timeout.in(5u, boost::bind(&echo::shutdown, this->shared_from_this()));
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

  boost::array<char, 1024u>     _buf;
  char const *                  _data_begin;
  char const *                  _data_end;

  LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);
};

#endif // IOXX_TEST_ECHO_HPP_INCLUDED_2010_02_23
