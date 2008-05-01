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

#ifndef IOXX_IOVEC_HPP_INCLUDED_2008_04_20
#define IOXX_IOVEC_HPP_INCLUDED_2008_04_20

#include <boost/range.hpp>
#include <boost/assert.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <iterator>
#include <sys/uio.h>

namespace ioxx
{
  /**
   * The native type used for scatter/gather I/O.
   */
  typedef ::iovec iovec;

  /**
   * Set an iovec to the given byte range.
   */
  inline void reset(iovec & iov, char * b, char * e)
  {
    BOOST_ASSERT(b <= e);
    iov.iov_base = b;
    iov.iov_len = static_cast<std::size_t>(e - b);
  }

  /**
   * Construct ioxx::iovec using an iterator range.
   */
  inline iovec make_iovec(char * b, char * e)
  {
    iovec v;
    reset(v, b, e);
    return v;
  }
}

namespace boost
{
#define IOXX_SPECIALIZE_IOVEC_TRAITS(t, mv, cv)                          \
  template<> struct range_ ## t<ioxx::iovec>       { typedef mv type; }; \
  template<> struct range_ ## t<ioxx::iovec const> { typedef cv type; }

  IOXX_SPECIALIZE_IOVEC_TRAITS(value,                   char,                                   char const);
  IOXX_SPECIALIZE_IOVEC_TRAITS(size,                    std::size_t,                            std::size_t);
  IOXX_SPECIALIZE_IOVEC_TRAITS(difference,              std::ptrdiff_t,                         std::ptrdiff_t);
  IOXX_SPECIALIZE_IOVEC_TRAITS(iterator,                char *,                                 char const *);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_iterator,          char const *,                           char const *);
  IOXX_SPECIALIZE_IOVEC_TRAITS(reverse_iterator,        std::reverse_iterator<char *>,          std::reverse_iterator<char const *>);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_reverse_iterator,  std::reverse_iterator<char const *>,    std::reverse_iterator<char const *>);
#undef IOXX_SPECIALIZE_IOVEC_TRAITS

  template<>
  inline range_size<ioxx::iovec>::type size(iovec const & iov)
  {
    return iov.iov_len;
  }

  template<>
  inline bool empty(iovec const & iov)
  {
    return iov.iov_len == 0u;
  }

  template<>
  inline range_iterator<ioxx::iovec>::type begin(iovec & iov)
  {
    return reinterpret_cast<range_iterator<ioxx::iovec>::type>(iov.iov_base);
  }

  template<>
  inline range_const_iterator<ioxx::iovec>::type begin(iovec const & iov)
  {
    return reinterpret_cast<range_const_iterator<ioxx::iovec>::type>(iov.iov_base);
  }

  template<>
  inline range_iterator<ioxx::iovec>::type end(iovec & iov)
  {
    return begin(iov) + size(iov);
  }

  template<>
  inline range_const_iterator<ioxx::iovec>::type end(iovec const & iov)
  {
    return const_begin(iov) + size(iov);
  }
}

#endif // IOXX_IOVEC_HPP_INCLUDED_2008_04_20
