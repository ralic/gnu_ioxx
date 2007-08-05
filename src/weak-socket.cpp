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

#include "ioxx/type/weak-socket.hpp"
#include "ioxx/type/system-error.hpp"
#include <boost/assert.hpp>
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <sys/types.h>
#include <unistd.h>             // close(2)
#include <fcntl.h>              // fcntl(2)
#include <sys/socket.h>         // setsockopt(2)
#include <netinet/in.h>         // sockaddr_in

using namespace std;

void ioxx::close(weak_socket s)
{
  BOOST_ASSERT(is_valid_weak_socket(s));
  for(unsigned int retries(3u); retries > 0; --retries)
  {
    int const rc( ::close(s) );
    if (rc == 0 || errno != EINTR)
      break;
  }
}

void ioxx::enable_blocking(weak_socket s, bool enable)
{
  BOOST_ASSERT(is_valid_weak_socket(s));
  int rc( fcntl(s, F_GETFL, 0) );
  if (rc == -1) throw system_error("cannot get socket flags");
  if (enable)   rc |=  O_NONBLOCK;
  else          rc &= ~O_NONBLOCK;
  rc = fcntl(s, F_SETFL, rc);
  if (rc == -1) throw system_error("cannot set modified socket flags");
}

char * ioxx::read(weak_socket s, char * begin, char const * end)
{
  BOOST_ASSERT(is_valid_weak_socket(s));
  BOOST_ASSERT(begin < end);
  size_t const len( end - begin );
  int const rc( ::read(s, begin, len) );
  if      (rc >  0)           return begin + rc;
  else if (rc == 0)           return NULL;
  else if (errno == EAGAIN)   return begin;
  else                        throw system_error("read() failed");
}

char const * ioxx::write(weak_socket s, char const * begin, char const * end)
{
  BOOST_ASSERT(is_valid_weak_socket(s));
  BOOST_ASSERT(begin < end);
  size_t const len( end - begin );
  int const rc( ::write(s, begin, len) );
  if      (rc > 0)           return begin + rc;
  else if (rc == 0)          return NULL;
  else if (errno == EAGAIN)  return begin;
  else                       throw system_error("write() failed");
}

char * ioxx::write(weak_socket s, char * begin, char const * end)
{
  return const_cast<char *>(write(s, const_cast<char const *>(begin), end));
}
