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
#include <boost/system/system_error.hpp>
#include <string>

#include <boost/config.hpp>
#ifdef _POSIX_SOURCE
#  include "os/posix.hpp"
#else
#  error "ioxx does not know this system"
#endif

namespace ioxx
{
  typedef native::iovec  iovec;
  typedef native::socket weak_socket;

  struct system_error : public boost::system::system_error
  {
    system_error();
    explicit system_error(std::string const & what);
  };

  /**
   *  \brief Read available input into a scattered memory buffer.
   *
   *  \param  s      socket to read from
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>s && begin &lt; end</code>
   *  \return number of \em bytes actually read; zero signifies end of input
   *  \throw  socket_error in case of an I/O error
   */
  byte_size read(weak_socket s, iovec * begin, iovec const * end);

  /**
   *  \brief Write a non-continuous memory buffer.
   *
   *  \param  s      socket to write to
   *  \param  begin  begin of iovector array
   *  \param  end    end of iovector array
   *  \pre    <code>s && begin &lt;= end</code>
   *  \return number of \em bytes actually written
   *  \throw  socket_error in case of an I/O error
   */
  byte_size write(weak_socket s, iovec const * begin, iovec const * end);
}

#endif // IOXX_OS_HPP_INCLUDED
