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

#ifndef IOXX_DETAIL_SHOW_HPP_INCLUDED_2008_04_20
#define IOXX_DETAIL_SHOW_HPP_INCLUDED_2008_04_20

#include <sstream>

namespace ioxx { namespace detail
{
  template <class T>
  inline std::string show(T const & val)
  {
    std::ostringstream os;
    os << val;
    return os.str();
  }
}}

#endif // IOXX_DETAIL_SHOW_HPP_INCLUDED_2008_04_20
