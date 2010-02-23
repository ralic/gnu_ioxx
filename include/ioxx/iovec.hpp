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

#ifndef IOXX_IOVEC_HPP_INCLUDED_2010_02_23
#define IOXX_IOVEC_HPP_INCLUDED_2010_02_23

#include <boost/range.hpp>
#include <boost/assert.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <iterator>
#include <limits>
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
  inline void reset(iovec & iov, char const * b, char const * e)
  {
    BOOST_ASSERT(b <= e);
    iov.iov_base = const_cast<char*>(b);
    iov.iov_len = static_cast<std::size_t>(e - b);
  }

  /**
   * Construct ioxx::iovec using an iterator range.
   */
  inline iovec make_iovec(char const * b, char const * e)
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
  inline range_difference<ioxx::iovec>::type size(iovec const & iov)
  {
    BOOST_ASSERT(iov.iov_len <= static_cast<std::size_t>(std::numeric_limits<range_difference<ioxx::iovec>::type>::max()));
    return static_cast<range_difference<ioxx::iovec>::type>(iov.iov_len);
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

#endif // IOXX_IOVEC_HPP_INCLUDED_2010_02_23
