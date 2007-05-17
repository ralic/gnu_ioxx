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

ioxx::byte_size ioxx::read( weak_socket             s
                          , iovec *                 begin
                          , iovec const *           end
                          , native::address *       peer_addr
                          , native::address_size *  peer_addr_len
                          , char const *            error_context
                          )
{
  BOOST_ASSERT(s >= 0);
  BOOST_ASSERT(begin < end);
  BOOST_ASSERT(peer_addr  || !peer_addr_len);
  BOOST_ASSERT(!peer_addr || (peer_addr_len && *peer_addr_len > 0));
  msghdr msg =
    { peer_addr
    , peer_addr_len ? *peer_addr_len : static_cast<native::address_size>(0)
    , begin
    , static_cast<size_t>(end - begin)
    , static_cast<void *>(0)                    // control data
    , static_cast<native::address_size>(0)      // control data size
    , static_cast<int>(0)                       // flags: set on return
    };
  ssize_t const rc( recvmsg(s, &msg, MSG_DONTWAIT) );
  if (rc < 0) throw system_error(error_context ? error_context : "read() failed");
  if (peer_addr_len) *peer_addr_len = msg.msg_namelen;
  return rc;
}

ioxx::byte_size ioxx::write( weak_socket             s
                           , iovec const *           begin
                           , iovec const *           end
                           , native::address *       peer_addr
                           , native::address_size    peer_addr_len
                           , char const *            error_context
                           )
{
  BOOST_ASSERT(s >= 0);
  BOOST_ASSERT(begin < end);
  BOOST_ASSERT(peer_addr  || !peer_addr_len);
  BOOST_ASSERT(!peer_addr || peer_addr_len > 0);
  msghdr msg =
    { peer_addr, peer_addr_len
    , const_cast<iovec *>(begin)
    , static_cast<size_t>(end - begin)
    , static_cast<void *>(0)                    // control data
    , static_cast<native::address_size>(0)      // control data size
    , static_cast<int>(0)                       // flags: set on return
    };
  ssize_t const rc( sendmsg(s, &msg, MSG_DONTWAIT | MSG_NOSIGNAL) );
  if (rc < 0)   throw system_error(error_context ? error_context : "write() failed");
  else          return rc;
}
