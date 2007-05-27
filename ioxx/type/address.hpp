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

#ifndef IOXX_TYPE_ADDRESS_HPP_INCLUDED
#define IOXX_TYPE_ADDRESS_HPP_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ioxx
{
  typedef sockaddr  address;
  typedef socklen_t address_size;
  typedef addrinfo  address_info;
}

#endif // IOXX_TYPE_ADDRESS_HPP_INCLUDED
