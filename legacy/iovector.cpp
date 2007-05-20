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

#include "ioxx/iovector.hpp"

#if 0                           // unstable and unfinished
std::size_t ioxx::read(socket s, system_iovector * begin, system_iovector const * end)
{
  I(s);
  I(begin < end);
  ::ssize_t const rc = ::readv(s.get(), begin, end - begin);
  if (rc < 0)   throw socket_error("reading", s);
  else          return static_cast<std::size_t>(rc);
}

std::size_t ioxx::write(socket s, system_iovector const * begin, system_iovector const * end)
{
  I(s);
  I(begin <= end);
  if (begin == end) return 0;
  ::ssize_t const rc = ::writev(s.get(), begin, end - begin);

  if      (rc == 0)             reset();
  else if (rc >  0)             begin += rc;
  else if (errno != EAGAIN)     throw socket_error("reading", *this);

  return begin;


  if (rc < 0) throw false;
  return rc;
}
#endif
