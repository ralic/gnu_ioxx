#ifndef IOXX_SOCKET_HPP_INCLUDED_2008_04_20
#define IOXX_SOCKET_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <iosfwd>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>

namespace ioxx
{
  typedef int native_socket_t;

  class socket : private boost::noncopyable
  {
  public:
    explicit socket(native_socket_t sock = -1) : _sock(std::max(sock, -1)) { }

    ~socket() { if (*this) close(); }

    void close()
    {
      BOOST_ASSERT(*this);
      IOXX_TRACE_SOCKET(_sock, "socket::close()");
      throw_errno_if_minus1("close(2)", boost::bind(&::close, _sock));
    }

    void set_nonblocking(bool enable = true)
    {
      BOOST_ASSERT(*this);
      IOXX_TRACE_SOCKET(_sock, (enable ? "enable" : "disable") << " nonblocking mode");
      int const rc( throw_errno_if_minus1("cannot obtain socket flags", boost::bind<int>(&::fcntl, _sock, F_GETFL, 0)) );
      int const flags( enable ? rc | O_NONBLOCK : rc & ~O_NONBLOCK );
      if (rc != flags)
        throw_errno_if_minus1("cannot set socket flags", boost::bind<int>(&::fcntl, _sock, F_SETFL, flags));
    }

    char * read(char * begin, char const * end)
    {
      BOOST_ASSERT(*this);
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "read(2)"
                                      , boost::bind(&::read, _sock, begin, static_cast<size_t>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "read(2) returned " << rc);
      switch (rc)
      {
        case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
        case 0:   return 0;
        default:  return begin + rc;
      };
    }

    char const * write(char const * begin, char const * end)
    {
      BOOST_ASSERT(*this);
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "write(2)"
                                      , boost::bind(&::write, _sock, begin, static_cast<size_t>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "write(2) returned " << rc);
      switch (rc)
      {
        case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
        case 0:   return 0;
        default:  return begin + rc;
      };
    }

    native_socket_t as_native_socket_t() const  { return _sock; }

    void            swap(socket & other)        { std::swap(_sock, other._sock); }
    void            reset(native_socket_t s)    { socket(s).swap(*this); }
    native_socket_t release()                   { native_socket_t s(_sock); _sock = -1; return s; }

    typedef native_socket_t (socket::*unspecified_bool_type)() const;
    operator unspecified_bool_type () const { return _sock >= 0 ? &socket::as_native_socket_t : 0; }

    friend bool operator<  (socket const & lhs, socket const & rhs) { return lhs._sock <  rhs._sock; }
    friend bool operator<= (socket const & lhs, socket const & rhs) { return lhs._sock <= rhs._sock; }
    friend bool operator== (socket const & lhs, socket const & rhs) { return lhs._sock == rhs._sock; }
    friend bool operator!= (socket const & lhs, socket const & rhs) { return lhs._sock != rhs._sock; }
    friend bool operator>= (socket const & lhs, socket const & rhs) { return lhs._sock >= rhs._sock; }
    friend bool operator>  (socket const & lhs, socket const & rhs) { return lhs._sock >  rhs._sock; }

    friend std::ostream & operator<< (std::ostream & os, socket const & s) { return os << "socket(" << s._sock << ')'; }

  private:
    native_socket_t _sock;
  };

} // namespace ioxx

namespace std
{
  template<>
  void swap<ioxx::socket>(ioxx::socket & lhs, ioxx::socket & rhs)
  {
    lhs.swap(rhs);
  }
}

#endif // IOXX_SOCKET_HPP_INCLUDED_2008_04_20
