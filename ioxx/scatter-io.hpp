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

#ifndef IOXX_SCATTER_IO_HPP_INCLUDED
#define IOXX_SCATTER_IO_HPP_INCLUDED

#include "type/weak-socket.hpp"
#include "type/iovec.hpp"
#include <utility>              // std::pair

namespace ioxx
{
  /// \brief A pointer to mutable ioxx::iovec.
  typedef iovec *               iovec_iterator;

  /// \brief A pointer to immutable ioxx::iovec.
  typedef iovec const *         iovec_const_iterator;

  /// \brief A byte iterator for mutable scatter/gather arrays.
  struct scatter_iterator : public std::pair<iovec_iterator, byte_iterator>
  {
    typedef std::pair<iovec_iterator, byte_iterator> base;

    scatter_iterator()                                      : base(0, 0)     { }
    scatter_iterator(iovec_iterator iov, byte_iterator pos) : base(iov, pos) { }
    scatter_iterator(base const & pair)                     : base(pair)     { }

    iovec_iterator  iov_ptr()  const { return first; }
    byte_iterator   byte_ptr() const { return second; }
  };

  /// \brief A byte iterator for immutable scatter/gather arrays.
  struct scatter_const_iterator : public std::pair<iovec_const_iterator, byte_const_iterator>
  {
    typedef std::pair<iovec_const_iterator, byte_const_iterator> base;

    scatter_const_iterator()                                           : base(0, 0)     { }
    scatter_const_iterator(iovec const * iov, byte_const_iterator pos) : base(iov, pos) { }
    scatter_const_iterator(scatter_iterator const & i)                 : base(i.iov_ptr(), i.byte_ptr()) { }
    scatter_const_iterator(base const & pair)                          : base(pair)     { }

    iovec const *       iov_ptr()  const { return first; }
    byte_const_iterator byte_ptr() const { return second; }
  };

  /**
   *  \brief Read available input into a scattered memory buffer.
   *
   *  \param  s      socket to read from
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>begin &lt; end</code>
   *  \throw  system_error in case of an I/O error
   *  \todo   need network address type in ioxx namespace
   */
  scatter_iterator read( weak_socket             s
                       , iovec_iterator          begin
                       , iovec_const_iterator    end
                       , system::address *       peer_addr     = 0
                       , system::address_size *  peer_addr_len = 0
                       , char const *            error_context = 0
                       );

  /**
   *  \brief Write a non-continuous memory buffer.
   *
   *  \param  s      socket to write to
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>begin &lt;= end</code>
   *  \throw  system_error in case of an I/O error
   *  \todo   need network address type in ioxx namespace
   */
  scatter_const_iterator write( weak_socket             s
                              , iovec_const_iterator    begin
                              , iovec_const_iterator    end
                              , system::address *       peer_addr     = 0
                              , system::address_size    peer_addr_len = 0
                              , char const *            error_context = 0
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
}

#endif // IOXX_SCATTER_IO_HPP_INCLUDED
