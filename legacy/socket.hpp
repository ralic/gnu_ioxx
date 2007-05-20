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

#ifndef IOXX_SOCKET_HPP_INCLUDED
#define IOXX_SOCKET_HPP_INCLUDED

#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include "sanity/invariant.hpp"
#include "sanity/platform.hpp"

namespace ioxx
{
  /**
   *  \brief Non-portable system socket type.
   */
  struct SANITY_DLL_EXPORT system_socket
  {
    int const fd;

    explicit system_socket(int s) : fd(s) { I(s >= 0); }
    ~system_socket() SANITY_NOTHROW_DECL;
  };

  /**
   *  \brief A portable socket type.
   */
  typedef boost::shared_ptr<system_socket const> socket;

  /**
   *  \brief Lift a POSIX file descriptor to a non-blocking socket.
   *  \throw system_error if the file descriptor is unusable
   */
  SANITY_DLL_EXPORT socket posix_socket(int fd = -1);

  inline std::ostream & operator<< (std::ostream & os, socket const & s)
  {
    return os << (s ? s->fd : -1);
  }

  typedef boost::uint32_t ipv4_address_t;
  typedef boost::uint16_t tcp_port_t;

  struct SANITY_DLL_EXPORT listen_socket : public socket
  {
    explicit listen_socket(socket const & s = socket()) : socket(s) { }
    listen_socket( tcp_port_t     port
                 , ipv4_address_t addr
                 , unsigned int   quelen
                 );

    socket accept();
  };

  struct SANITY_DLL_EXPORT data_socket : public socket
  {
    explicit data_socket(socket const & s = socket()) : socket(s) { }

    char *       read(char * begin, char const * end);
    char const * write(char const * begin, char const * end);
    char *       write(char * begin, char const * end);
  };

} // namespace ioxx

#endif // IOXX_SOCKET_HPP_INCLUDED
