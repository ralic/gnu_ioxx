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

#include <ioxx/core.hpp>

#include <ioxx/acceptor.hpp>
#include <boost/enable_shared_from_this.hpp>
// #include <boost/scoped_ptr.hpp>
// #include <boost/array.hpp>

template <class IOCore>
class httpd : public boost::enable_shared_from_this< httpd<IOCore> >
{
public:
  typedef IOCore                        io_core;
  typedef httpd<io_core>                self;
  typedef boost::shared_ptr<self>       self_ptr;
  typedef typename io_core::timeout     timeout;
  typedef typename io_core::event_set   event_set;
  typedef typename io_core::socket      socket;
  typedef typename io_core::handler     handler;
  typedef typename socket::address      address;
  typedef typename socket::native_t     native_socket_t;

  static void accept(io_core & io, native_socket_t s, address const & peer)
  {
    self_ptr p;
    p.reset(new httpd(io, s));
    p->_sock.modify(boost::bind(&self::run, p, _1), socket::readable);;
    p->_timeout.in(10u, boost::bind(&socket::modify, boost::ref(p->_sock), handler()));
  }

private:
  httpd(io_core & io, native_socket_t s) : _sock(io, s), _timeout(io)
  {
  }

  void run(event_set ev)
  {
  }

  socket        _sock;
  timeout       _timeout;
};

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/pool/pool_alloc.hpp>

static sig_atomic_t stop_service = false;
static void stop_service_hook(int) { stop_service = true; }

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  typedef boost::fast_pool_allocator<char>              allocator;
  typedef ioxx::core<allocator>                         io_core;
  typedef ioxx::acceptor<allocator>                     acceptor;
  typedef acceptor::endpoint                            endpoint;

  using boost::bind;
  using boost::ref;

  ioxx::block_signals no_signal_scope;

  io_core io;
  acceptor listen_port(io, endpoint("127.0.0.1", "8080"), bind(&httpd<io_core>::accept, ref(io), _1, _2) );
  for (int i(0); i != 32; ++i) ::signal(i, SIG_IGN);
  ioxx::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGINT, &stop_service_hook));
  ioxx::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGTERM, &stop_service_hook));
  io.in(5u, bind(&stop_service_hook, 0));
  for (ioxx::seconds_t timeout( io.run() ); !stop_service; timeout = io.run())
  {
    io.wait(timeout);
  }
}
