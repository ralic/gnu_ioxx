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
#include <boost/range.hpp>
#include "sanity/invariant.hpp"

namespace ioxx
{
  template <class range_type, class iterator>
  inline bool reset_begin(range_type & buf, iterator new_begin)
  {
    iterator const beg( boost::begin(buf) );
    iterator const end( boost::end(buf) );
    I(new_begin <= end);
    if (new_begin == beg) return false;
    buf = range_type(new_begin, end);
    return true;
  }

  template <class range_type, class iterator>
  inline bool reset_end(range_type & buf, iterator new_end)
  {
    iterator const beg( boost::begin(buf) );
    iterator const end( boost::end(buf) );
    I(beg <= new_end);
    if (new_end == end) return false;
    buf = range_type(beg, new_end);
    return true;
  }

  template <class range_type, class iterator>
  inline bool move_down(range_type & buf, iterator new_begin)
  {
    iterator const beg( boost::begin(buf) );
    iterator const end( boost::end(buf) );
    I(new_begin <= beg); I(beg <= end);
    if (new_begin < beg)
    {
      buf = range_type(new_begin, std::copy(beg, end, new_begin));
      return true;
    }
    else
      return false;
  }

  template <class callback, class range_type>
  void consume(range_type & buf, callback f = callback())
  {
    while(!boost::empty(buf) && reset_begin(buf, f(boost::begin(buf), boost::end(buf))))
      ;
  }

} // namespace ioxx

#endif // IOXX_MEMORY_HPP_INCLUDED
