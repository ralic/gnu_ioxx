/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_TYPE_WEAK_SOCKET_INCLUDED
#define IOXX_TYPE_WEAK_SOCKET_INCLUDED

namespace ioxx
{
  /// \brief The native system socket type.
  typedef int weak_socket;

  /// \brief Construct an invalid ioxx::weak_socket.
  inline weak_socket invalid_weak_socket()
  {
    return -1;
  }

  /// \brief Test whether a given ioxx::weak_socket is valid.
  inline bool is_valid_weak_socket(weak_socket s)
  {
    return s >= 0;
  }
}

#endif // IOXX_TYPE_WEAK_SOCKET_INCLUDED
