#ifndef IOXX_SOCKET_HPP_INCLUDED_2008_04_20
#define IOXX_SOCKET_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <unistd.h>

namespace ioxx
{
  typedef int socket_t;

  inline void close(socket_t s, std::string const & error_msg)
  {
    IOXX_TRACE_SOCKET(s, "close(2)");
    throw_errno_if_minus1(error_msg, boost::bind(&::close, s));
  }
}

#endif // IOXX_SOCKET_HPP_INCLUDED_2008_04_20
