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

#ifndef IOXX_OS_POSIX_HPP_INCLUDED
#define IOXX_OS_POSIX_HPP_INCLUDED

#ifndef IOXX_OS_HPP_INCLUDED
#  error "include ioxx/os.hpp instead of this file"
#endif

#include <boost/assert.hpp>
#include <sys/types.h>
#include <sys/uio.h>

namespace ioxx { namespace native
{
  using ::iovec;
  typedef int socket;
}}

namespace boost
{
#define IOXX_SPECIALIZE_IOVEC_TRAITS(t, mv, cv)                                         \
  template<> struct range_ ## t<ioxx::native::iovec>       { typedef ioxx::mv type; };  \
  template<> struct range_ ## t<ioxx::native::iovec const> { typedef ioxx::cv type; }
  IOXX_SPECIALIZE_IOVEC_TRAITS(value,                   byte_type,                    byte_type const);
  IOXX_SPECIALIZE_IOVEC_TRAITS(size,                    byte_size,                    byte_size);
  IOXX_SPECIALIZE_IOVEC_TRAITS(difference,              byte_offset,                  byte_offset);
  IOXX_SPECIALIZE_IOVEC_TRAITS(iterator,                byte_iterator,                byte_const_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_iterator,          byte_const_iterator,          byte_const_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(reverse_iterator,        byte_reverse_iterator,        byte_const_reverse_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_reverse_iterator,  byte_const_reverse_iterator,  byte_const_reverse_iterator);
#undef IOXX_SPECIALIZE_IOVEC_TRAITS

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
  inline iovec & reset_begin(iovec & iov, byte_iterator b)
  {
    iov.iov_base = b;
    return iov;
  }

  template<>
  inline iovec & reset_end(iovec & iov, byte_iterator e)
  {
    BOOST_ASSERT(begin(iov) <= e);
    iov.iov_len = e - begin(iov);
    return iov;
  }
}

#endif // IOXX_OS_POSIX_HPP_INCLUDED
