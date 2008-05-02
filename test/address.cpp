/*
 * Copyright (c) 2008 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Copying and distribution of this file, with or without modification, are
 * permitted in any medium without royalty provided the copyright notice and
 * this notice are preserved.
 */

#include "ioxx/type/system-error.hpp"
#include "ioxx/probe.hpp"
#include "ioxx/config.hpp"
#include <sys/types.h>          // getaddrinfo() from POSIX.1-2001, RFC 2553
#include <sys/socket.h>
#include <netdb.h>
#include <boost/shared_ptr.hpp>

#define TRACE_ARG1(arg)         IOXX_TRACE_MSG(" " #arg " = " << arg)
#define TRACE_ARG2(arg1, arg2)  IOXX_TRACE_MSG(arg1 << ", " #arg2 " = " << arg2)

class peer_address
{
  boost::shared_ptr< ::addrinfo > _addr;
  int                             _errc;

public:
  typedef std::pair<std::string,std::string> string_pair;

  peer_address() { }

  peer_address(char const * address, char const * service)
  {
    TRACE_ARG2(address, service);
    ::addrinfo   hint = { AI_NUMERICHOST, 0, 0, 0, 0u, NULL, NULL, NULL };
    ::addrinfo * addr;
    _errc = ::getaddrinfo(address, service, &hint, &addr);
    if (_errc == 0) _addr.reset( addr, ::freeaddrinfo );
  }

  bool          good()  const { return _addr; }
  char const *  error() const { return good() ? NULL : ::gai_strerror(_errc); }

  string_pair show() const
  {
    IOXX_ASSERT(good());
    char host[NI_MAXHOST], service[NI_MAXSERV];
    int const rc( ::getnameinfo( _addr->ai_addr, _addr->ai_addrlen
                               , host, sizeof(host), service, sizeof(service)
                               , NI_NUMERICHOST
                               )
                );
    if (rc != 0) throw ioxx::system_error("getnameinfo(3)");
    return std::make_pair(host, service);
  }
};

// ----- test code ------------------------------------------------------------

int main(int argc, char** argv)
{
  peer_address addr;
  switch (argc)
  {
    case 3:
      addr = peer_address(argv[1], argv[2]);
      break;

    default:
      addr = peer_address("195.234.152.69", "http");
  }
  if (addr.good())
    IOXX_TRACE_MSG(" addr.show = " << addr.show().first << " : " << addr.show().second);
  else
    IOXX_TRACE_MSG(" addr.error = " << addr.error());

  IOXX_TRACE_MSG(" terminating gracefully");
  return 0;
}
