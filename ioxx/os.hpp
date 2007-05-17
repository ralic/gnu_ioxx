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

#ifndef IOXX_OS_HPP_INCLUDED
#define IOXX_OS_HPP_INCLUDED

#include "memory.hpp"
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/config.hpp>
#ifdef _POSIX_SOURCE
#  include "os/posix.hpp"
#else
#  error "ioxx does not know this system"
#endif

namespace ioxx
{
  typedef native::socket        weak_socket;
  typedef native::iovec         iovec;
  typedef iovec *               iovec_iterator;
  typedef iovec const *         iovec_const_iterator;

  struct scatter_iterator : public std::pair<iovec_iterator, byte_iterator>
  {
    typedef std::pair<iovec_iterator, byte_iterator> base;

    scatter_iterator()                                      : base(0, 0)     { }
    scatter_iterator(iovec_iterator iov, byte_iterator pos) : base(iov, pos) { }
    scatter_iterator(base const & pair)                     : base(pair)     { }

    iovec_iterator  iov_ptr()  const { return first; }
    byte_iterator   byte_ptr() const { return second; }
  };

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

  struct system_error : public std::runtime_error
  {
    system_error();
    explicit system_error(std::string const & context);
  };

  /**
   *  \brief Read available input into a scattered memory buffer.
   *
   *  \param  s      socket to read from
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>begin &lt; end</code>
   *  \throw  system_error in case of an I/O error
   */
  scatter_iterator read( weak_socket             s
                       , iovec_iterator          begin
                       , iovec_const_iterator    end
                       , native::address *       peer_addr     = 0
                       , native::address_size *  peer_addr_len = 0
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
   */
  scatter_const_iterator write( weak_socket s
                              , iovec_const_iterator    begin
                              , iovec_const_iterator    end
                              , native::address *       peer_addr     = 0
                              , native::address_size    peer_addr_len = 0
                              , char const *            error_context = 0
                              );
}

#endif // IOXX_OS_HPP_INCLUDED
