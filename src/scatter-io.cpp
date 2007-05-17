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

#include "ioxx/os.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/assert.hpp>

using namespace std;

ioxx::byte_size ioxx::read(weak_socket s, iovec * begin, iovec const * end)
{
  BOOST_ASSERT(s >= 0); BOOST_ASSERT(begin < end);
  msghdr msg =
    { static_cast<native::address *>(0)
    , static_cast<native::address_size>(0)
    , const_cast<iovec *>(begin)
    , static_cast<size_t>(end - begin)
    , static_cast<void *>(0)                    // control data
    , static_cast<native::address_size>(0)      // control data size
    , static_cast<int>(0)                       // flags: set on return
    };
  ssize_t const rc( recvmsg(s, &msg, MSG_DONTWAIT) );
  if (rc < 0)   throw system_error("write() failed:");
  else          return rc;

}

ioxx::byte_size ioxx::write(weak_socket s, iovec const * begin, iovec const * end)
{
  BOOST_ASSERT(s >= 0); BOOST_ASSERT(begin < end);
  msghdr msg =
    { static_cast<native::address *>(0)
    , static_cast<native::address_size>(0)
    , const_cast<iovec *>(begin)
    , static_cast<size_t>(end - begin)
    , static_cast<void *>(0)                    // control data
    , static_cast<native::address_size>(0)      // control data size
    , static_cast<int>(0)                       // flags: set on return
    };
  ssize_t const rc( sendmsg(s, &msg, MSG_DONTWAIT | MSG_NOSIGNAL) );
  if (rc < 0)   throw system_error("write() failed:");
  else          return rc;
}
