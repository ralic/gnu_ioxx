/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_TYPE_IOVEC_HPP_INCLUDED
#define IOXX_TYPE_IOVEC_HPP_INCLUDED

#include "byte-range.hpp"
#include <sys/types.h>
#include <sys/uio.h>

namespace ioxx
{
  typedef ::iovec iovec;
}

namespace boost
{
#define IOXX_SPECIALIZE_IOVEC_TRAITS(t, mv, cv)                                 \
  template<> struct range_ ## t<ioxx::iovec>       { typedef ioxx::mv type; };  \
  template<> struct range_ ## t<ioxx::iovec const> { typedef ioxx::cv type; }

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
  /// \brief Set ioxx::iovec to the given byte range.
  inline void reset(iovec & iov, byte_iterator b, byte_iterator e)
  {
    BOOST_ASSERT(b <= e);
    iov.iov_base = b;
    iov.iov_len = static_cast<byte_size>(e - b);
  }

  /// \brief Construct ioxx::iovec using an iterator range.
  inline iovec make_iovec(byte_iterator begin, byte_iterator end)
  {
    iovec v;
    reset(v, begin, end);
    return v;
  }
}

#endif // IOXX_TYPE_IOVEC_HPP_INCLUDED
