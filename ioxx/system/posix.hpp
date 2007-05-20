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

#ifndef IOXX_SYSTEM_POSIX_HPP_INCLUDED
#define IOXX_SYSTEM_POSIX_HPP_INCLUDED

#ifndef IOXX_SYSTEM_HPP_INCLUDED
#  error "include ioxx/system.hpp instead of this file"
#endif

#include <boost/assert.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netdb.h>

namespace ioxx
{
  namespace system
  {
    using ::iovec;
    typedef int       socket;
    typedef sockaddr  address;
    typedef addrinfo  address_info;
    typedef socklen_t address_size;
  }
}

#include "iovec-traits.hpp"

namespace boost
{
  template<>
  inline ioxx::byte_size size(iovec const & iov)
  {
    return iov.iov_len;
  }

  template<>
  inline bool empty(iovec const & iov)
  {
    return iov.iov_len == 0u;
  }

  template<>
  inline ioxx::byte_iterator begin(iovec & iov)
  {
    return reinterpret_cast<ioxx::byte_iterator>(iov.iov_base);
  }

  template<>
  inline ioxx::byte_const_iterator begin(iovec const & iov)
  {
    return reinterpret_cast<ioxx::byte_iterator>(iov.iov_base);
  }

  template<>
  inline ioxx::byte_iterator end(iovec & iov)
  {
    return begin(iov) + size(iov);
  }

  template<>
  inline ioxx::byte_const_iterator end(iovec const & iov)
  {
    return const_begin(iov) + size(iov);
  }
}

namespace ioxx
{
  template<>
  inline void reset_begin(iovec & iov, byte_iterator b)
  {
    iov.iov_base = b;
  }

  template<>
  inline void reset_end(iovec & iov, byte_iterator e)
  {
    BOOST_ASSERT(begin(iov) <= e);
    iov.iov_len = e - begin(iov);
  }
}

#endif // IOXX_SYSTEM_POSIX_HPP_INCLUDED
