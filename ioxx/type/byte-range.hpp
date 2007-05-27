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

#ifndef IOXX_TYPE_BYTE_RANGE_HPP_INCLUDED
#define IOXX_TYPE_BYTE_RANGE_HPP_INCLUDED

#include "byte.hpp"
#include <boost/range.hpp>
#include <iterator>

namespace ioxx
{
  /// \brief An iterator over mutable bytes.
  typedef byte_type *                                   byte_iterator;

  /// \brief An iterator over immutable bytes.
  typedef byte_type const *                             byte_const_iterator;

  /// \brief A direction-reversed iterator over mutable bytes.
  typedef std::reverse_iterator<byte_iterator>          byte_reverse_iterator;

  /// \brief A direction-reversed iterator over immutable bytes.
  typedef std::reverse_iterator<byte_const_iterator>    byte_const_reverse_iterator;

  using boost::iterator_range;
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

} // namespace ioxx

#endif // IOXX_TYPE_BYTE_RANGE_HPP_INCLUDED
