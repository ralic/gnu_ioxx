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

#ifndef IOXX_IOVECTOR_HPP_INCLUDED
#define IOXX_IOVECTOR_HPP_INCLUDED

#include <iterator>             // define std::reverse_iterator
#include <algorithm>            // define std::swap()
#include "socket.hpp"

/**
 *  \def   IOXX_SYSTEM_IOVECTOR
 *  \brief Expands to the system's native io vector type.
 */
#ifdef SANITY_POSIX_SOURCE
#  include <sys/uio.h>          // POSIX 1003.1-2001
#  define IOXX_SYSTEM_IOVECTOR ::iovec
#endif

namespace ioxx
{
  /// \brief Our native io vector type.
  typedef IOXX_SYSTEM_IOVECTOR system_iovector;

  /**
   *  \brief A memory buffer suitable for scatter I/O.
   *
   *  This class template provides a mutable, random-access container
   *  interface to your system's native io vector type. On Posix, this
   *  data structure looks as follows:
   *
   *  <pre>
   *   struct iovec
   *   {
   *      void * iov_base;   // starting address
   *      size_t iov_len;    // number of bytes
   *   };
   *  </pre>
   *
   *  We have to expose the native data type because scatter I/O
   *  operates on arrays of it.
   *
   *  \note If you are getting linker errors concerning a missing \c
   *        char_traits class when using this header, include \c
   *        <code>&lt;ioxx/iovector_traits.hpp&gt;</code> as well.
   */
  template <class charT = char>
  struct iovector : public system_iovector
  {
    typedef charT                                 value_type;
    typedef value_type *                          pointer;
    typedef value_type const *                    const_pointer;
    typedef pointer                               iterator;
    typedef const_pointer                         const_iterator;
    typedef value_type &                          reference;
    typedef value_type const &                    const_reference;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::size_t                           size_type;
    typedef std::ptrdiff_t                        difference_type;
    typedef iterator (iovector::*unspecified_bool_type) () const;

    iovector(pointer base, size_type len)
    {
      I(base || len == 0u);
      iov_base = const_cast<void *>(static_cast<void const *>(base));
      iov_len  = len * sizeof(value_type);
    }

    iovector()                                  { reset(NULL, 0u); }
    iovector(pointer b, const_pointer e)        { I(b <= e); reset(b, e - b); }

    void reset(pointer b, size_type n)          { *this = iovector(b, n);     }
    void swap(iovector & other)                 { std::swap(*this, other); }
    void resize(size_type len)                  { reset(begin(), len);     }

    pointer                base()     const     { return static_cast<pointer>(iov_base); }
    const_pointer          data()     const     { return base(); }
    size_type              size()     const     { return (iov_len / sizeof(value_type)); }
    size_type              max_size() const     { return size(); }
    bool                   empty()    const     { return data() && size(); }

    operator unspecified_bool_type () const     { return empty() ? &iovector::begin : 0; }

    iterator               begin()              { return base(); }
    const_iterator         begin()    const     { return base(); }
    iterator               end()                { return (begin() + size()); }
    const_iterator         end()      const     { return (begin() + size()); }
    reverse_iterator       rbegin()             { return reverse_iterator(end());         }
    const_reverse_iterator rbegin()   const     { return const_reverse_iterator(end());   }
    reverse_iterator       rend()               { return reverse_iterator(begin());       }
    const_reverse_iterator rend()     const     { return const_reverse_iterator(begin()); }

    reference       operator[] (size_type n)       { I(n < size()); return begin()[n]; }
    const_reference operator[] (size_type n) const { I(n < size()); return begin()[n]; }
  };

  /**
   *  \brief Read available input data into memory designated by iovectors.
   *
   *  \param  s      socket to read from
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>s && begin &lt; end</code>
   *  \return number of \em bytes actually read; zero signifies end of input
   *  \throw  socket_error in case of an I/O error
   */
  std::size_t read(socket s, system_iovector * begin, system_iovector const * end);

  /**
   *  \brief Write data designated by iovector array to socket.
   *
   *  \param  s      socket to write to
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>s && begin &lt;= end</code>
   *  \return number of \em bytes actually written
   *  \throw  socket_error in case of an I/O error
   */
  std::size_t write(socket s, system_iovector const * begin, system_iovector const * end);

} // namespace ioxx

#endif // IOXX_IOVECTOR_HPP_INCLUDED
