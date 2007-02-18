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

#ifndef IOXX_IOVECTOR_TRAITS_HPP_INCLUDED
#define IOXX_IOVECTOR_TRAITS_HPP_INCLUDED

#include "iovector.hpp"
#include <string>

namespace std                   /// ISO C++ components.
{
  /**
   *  \brief Specialize \c std::char_traits<iovector>.
   *
   *  Older compilers need this to instantiate a \c std::basic_string
   *  of iovectors.
   */
  template <class charT>
  struct char_traits< ioxx::iovector<charT> >
  {
    typedef ioxx::iovector<charT>       char_type;
    typedef char_type                   iovector;

    static void	assign(iovector & dst, iovector const & src)
    {
      dst = src;
    }

    static bool eq(iovector const & lhs, iovector const & rhs)
    {
      return lhs == rhs;
    }

    static iovector * assign(iovector * dst, size_t n, iovector const & v)
    {
      return std::fill(dst, dst + n, v);
    }

    static iovector * move(iovector * dst, iovector const * src, size_t n)
    {
      return static_cast<iovector*>(std::memmove(dst, src, n * sizeof(iovector)));
    }

    static iovector * copy(iovector * dst, iovector const * src, size_t n)
    {
      return static_cast<iovector*>(std::memcpy(dst, src, n * sizeof(iovector)));
    }
  };
}


#endif // IOXX_IOVECTOR_TRAITS_HPP_INCLUDED
