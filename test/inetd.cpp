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

#include <ioxx/core.hpp>
#include <ioxx/acceptor.hpp>
#include "daytime.hpp"
#include "echo.hpp"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/pool/pool_alloc.hpp>

static sig_atomic_t stop_service = false;
static void stop_service_hook(int) { stop_service = true; }

BOOST_AUTO_TEST_CASE( example_inetd )
{
  typedef boost::fast_pool_allocator<char>              allocator;
  typedef ioxx::core<allocator>                         io_core;
  typedef ioxx::acceptor<allocator>                     acceptor;
  typedef acceptor::socket                              socket;
  typedef acceptor::endpoint                            endpoint;

  using boost::bind;
  using boost::ref;

  // Allow signal delivery only during io_core::wait().
  ioxx::signal_block no_signal_scope;

  // Ignore all signals except SIGINT and SIGTERM.
  for (int i(0); i != 32; ++i) ::signal(i, SIG_IGN);
  ioxx::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGINT, &stop_service_hook));
  ioxx::throw_errno_if(boost::bind(std::equal_to<sighandler_t>(), _1, SIG_ERR), "signal(2)", bind(&::signal, SIGTERM, &stop_service_hook));

  // The main i/o event dispatcher.
  io_core io;

  // Accept daytime TCP service.
  acceptor daytime_tcp( io, endpoint("127.0.0.1", "8080", socket::stream_service)
                      , bind(&daytime<io_core>::accept, ref(io), _1, _2)
                      );

  // Accept echo TCP service.
  acceptor echo_tcp( io, endpoint("127.0.0.1", "8081", socket::stream_service)
                   , bind(&echo<io_core>::accept, ref(io), _1, _2)
                   );

  // Shut everything down after 5 seconds.
  io.in(5, bind(&stop_service_hook, 0));

  // The main i/o loop.
  for (ioxx::seconds_t timeout( io.run() ); !stop_service; timeout = io.run())
  {
    io.wait(timeout);
  }
}
