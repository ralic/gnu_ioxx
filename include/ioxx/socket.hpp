#ifndef IOXX_SOCKET_HPP_INCLUDED_2008_04_20
#define IOXX_SOCKET_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <unistd.h>
#include <fcntl.h>

namespace ioxx
{
  typedef int socket_t;

  inline void close(socket_t s, std::string const & error_msg)
  {
    IOXX_TRACE_SOCKET(s, "close(2)");
    throw_errno_if_minus1(error_msg, boost::bind(&::close, s));
  }

  inline void nonblocking(socket_t s, bool enable = true)
  {
    IOXX_TRACE_SOCKET(s, (enable ? "enable" : "disable") << " nonblocking mode");
    BOOST_ASSERT(s >= 0);
    int const rc( throw_errno_if_minus1("cannot obtain socket flags", boost::bind<int>(&::fcntl, s, F_GETFL, 0)) );
    int const flags( enable ? rc | O_NONBLOCK : rc & ~O_NONBLOCK );
    if (rc != flags)
      throw_errno_if_minus1("cannot set socket flags", boost::bind<int>(&::fcntl, s, F_SETFL, flags));
  }

  struct not_ewould_block : public std::unary_function<ssize_t, bool>
  {
    bool operator() (ssize_t rc) const
    {
      return rc < 0 && errno != EWOULDBLOCK && errno != EAGAIN;
    }
  };

  inline char * read(socket_t s, char * begin, char const * end)
  {
    BOOST_ASSERT(begin < end);
    ssize_t const rc( throw_errno_if( not_ewould_block()
                                    , "read(2)"
                                    , boost::bind(&::read, s, begin, static_cast<size_t>(end - begin))
                                    ));
    IOXX_TRACE_SOCKET(s, "read(2) returned " << rc);
    switch (rc)
    {
      case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
      case 0:   return 0;
      default:  return begin + rc;
    };
  }

  inline char const * write(socket_t s, char const * begin, char const * end)
  {
    BOOST_ASSERT(begin < end);
    ssize_t const rc( throw_errno_if( not_ewould_block()
                                    , "write(2)"
                                    , boost::bind(&::write, s, begin, static_cast<size_t>(end - begin))
                                    ));
    IOXX_TRACE_SOCKET(s, "write(2) returned " << rc);
    switch (rc)
    {
      case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
      case 0:   return 0;
      default:  return begin + rc;
    };
  }

} // namespace ioxx

#endif // IOXX_SOCKET_HPP_INCLUDED_2008_04_20
