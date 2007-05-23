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
  /// \brief Bytes are signed or unsigned, we don't know.
  typedef char                                          byte_type;

  /// \brief An unsigned number of bytes.
  typedef std::size_t                                   byte_size;

  /// \brief A signed distance between two byte addresses.
  typedef std::ptrdiff_t                                byte_offset;

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

  /**
   *  \pre  <code>b &lt;= e</code>
   */
  template<class Range>
  inline void reset( Range & iov
                   , typename range_iterator<Range>::type b
                   , typename range_iterator<Range>::type e
                   );

  /**
   *  \brief Advance iterators into a paged container by a value offset.
   */
  template <class page_iterator, class value_iterator, class size_type>
  inline void paged_advance( page_iterator &    page_iter
                           , value_iterator &   val_iter
                           , size_type          i
                           )
  {
    size_type n;
    while (i)
    {
      n = std::distance(val_iter, end(*page_iter));
      if (i > n)
      {
        i -= n;
        ++page_iter;
        val_iter = begin(*page_iter);
      }
      else
        return std::advance(val_iter, i);
    }
  }

} // namespace ioxx

#endif // IOXX_MEMORY_HPP_INCLUDED
