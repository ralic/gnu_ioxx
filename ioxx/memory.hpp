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

#ifndef IOXX_MEMORY_HPP_INCLUDED
#define IOXX_MEMORY_HPP_INCLUDED

#include <algorithm>
#include <iterator>
#include <boost/range.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>

namespace ioxx
{
  typedef char                                          byte_type;
  typedef std::size_t                                   byte_size;
  typedef std::ptrdiff_t                                byte_offset;
  typedef byte_type *                                   byte_iterator;
  typedef byte_type const *                             byte_const_iterator;
  typedef std::reverse_iterator<byte_iterator>          byte_reverse_iterator;
  typedef std::reverse_iterator<byte_const_iterator>    byte_const_reverse_iterator;

  using boost::range_value;
  using boost::range_size;
  using boost::range_difference;
  using boost::range_iterator;
  using boost::range_const_iterator;
  using boost::range_reverse_iterator;
  using boost::range_const_reverse_iterator;
  using boost::range_result_iterator;
  using boost::sub_range;

  using boost::size;
  using boost::empty;
  using boost::begin;
  using boost::const_begin;
  using boost::rbegin;
  using boost::const_rbegin;
  using boost::end;
  using boost::const_end;
  using boost::rend;
  using boost::const_rend;

  template<class range>
  inline range & reset_begin(range & iov, typename range_iterator<range>::type);

  template<class range>
  inline range & reset_end(range & iov, typename range_iterator<range>::type);

  template<class range>
  inline range & reset(range & iov, typename range_iterator<range>::type b, typename range_iterator<range>::type e)
  {
    return reset_end(reset_begin(iov, b), e);
  }

} // namespace ioxx

#endif // IOXX_MEMORY_HPP_INCLUDED
