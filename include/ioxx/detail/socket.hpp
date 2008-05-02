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

#ifndef IOXX_DETAIL_SOCKET_HPP_INCLUDED_2008_04_20
#define IOXX_DETAIL_SOCKET_HPP_INCLUDED_2008_04_20

#include "config.hpp"
#include "error.hpp"
#include <boost/noncopyable.hpp>
#include <iosfwd>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ioxx { namespace detail
{
  class socket : private boost::noncopyable
  {
  public:
    class address
    {
    public:
      typedef char host_name[NI_MAXHOST];
      typedef char service_name[NI_MAXSERV];

      address() : _len(0) { }
      address(sockaddr const & addr, socklen_t len) : _addr(addr), _len(len) { }

      void show(host_name & host, service_name & service) const
      {
        int const rc( getnameinfo(&_addr, _len, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) );
        if (rc != 0) throw std::runtime_error(gai_strerror(rc));
      }

      std::string show() const
      {
        host_name host;
        service_name service;
        show(host, service);
        return std::string(host) + ':' + service;
      }

      sockaddr const &   as_sockaddr()  const { return _addr; }
      socklen_t const &  as_socklen_t() const { return _len; }

    private:
      sockaddr  _addr;
      socklen_t _len;
    };

    enum ownership_type_tag { weak, take_ownership };

    explicit socket(native_socket_t sock, ownership_type_tag owner = take_ownership) : _sock(sock)
    {
      IOXX_TRACE_SOCKET(_sock, "construct " << this);
      if (_sock < 0) throw std::invalid_argument("cannot construct an invalid ioxx::socket");
      close_on_destruction(owner == take_ownership);
    }

    ~socket()
    {
      IOXX_TRACE_SOCKET(_sock, (_close_on_destruction ? "close and " : "") << "destruct " << this) ;
      if (_close_on_destruction)
        throw_errno_if_minus1("close(2)", boost::bind(&::close, _sock));
    }

    void close_on_destruction(bool enable = true)
    {
      IOXX_TRACE_SOCKET(_sock, (enable ? "enable" : "disable") << " close-on-destruction semantics on " << this);
      _close_on_destruction = enable;
    }

    bool close_on_destruction() const
    {
      return _close_on_destruction;
    }

    void set_nonblocking(bool enable = true)
    {
      IOXX_TRACE_SOCKET(_sock, (enable ? "enable" : "disable") << " nonblocking mode on " << this);
      int const rc( throw_errno_if_minus1("cannot obtain socket flags", boost::bind<int>(&::fcntl, _sock, F_GETFL, 0)) );
      int const flags( enable ? rc | O_NONBLOCK : rc & ~O_NONBLOCK );
      if (rc != flags)
        throw_errno_if_minus1("cannot set socket flags", boost::bind<int>(&::fcntl, _sock, F_SETFL, flags));
    }

    char * read(char * begin, char const * end)
    {
      IOXX_TRACE_SOCKET(_sock, "read up to " << end - begin << " bytes from " << this);
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "read(2)"
                                      , boost::bind(&::read, _sock, begin, static_cast<size_t>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "read(2) received " << rc << " bytes from " << this);
      switch (rc)
      {
        case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
        case 0:   return 0;
        default:  return begin + rc;
      };
    }

    char const * write(char const * begin, char const * end)
    {
      IOXX_TRACE_SOCKET(_sock, "write up to " << end - begin << " bytes to " << this);
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "write(2)"
                                      , boost::bind(&::write, _sock, begin, static_cast<size_t>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "write(2) sent " << rc << " bytes to " << this);
      switch (rc)
      {
        case -1:  BOOST_ASSERT(errno == EWOULDBLOCK || errno == EAGAIN); return begin;
        case 0:   return 0;
        default:  return begin + rc;
      };
    }

    address local_address() const
    {
      sockaddr addr;
      socklen_t len( sizeof(addr) );
      throw_errno_if_minus1("getsockname(2)", boost::bind(&::getsockname, _sock, &addr, &len));
      return address(addr, len);
    }

    address peer_address() const
    {
      sockaddr addr;
      socklen_t len( sizeof(addr) );
      throw_errno_if_minus1("getpeername(2)", boost::bind(&::getpeername, _sock, &addr, &len));
      return address(addr, len);
    }

    native_socket_t as_native_socket_t() const { return _sock; }

    friend bool operator<  (socket const & lhs, socket const & rhs) { return lhs._sock <  rhs._sock; }
    friend bool operator<= (socket const & lhs, socket const & rhs) { return lhs._sock <= rhs._sock; }
    friend bool operator== (socket const & lhs, socket const & rhs) { return lhs._sock == rhs._sock; }
    friend bool operator!= (socket const & lhs, socket const & rhs) { return lhs._sock != rhs._sock; }
    friend bool operator>= (socket const & lhs, socket const & rhs) { return lhs._sock >= rhs._sock; }
    friend bool operator>  (socket const & lhs, socket const & rhs) { return lhs._sock >  rhs._sock; }

    friend std::ostream & operator<< (std::ostream & os, socket const & s) { return os << "socket(" << s._sock << ')'; }

  private:
    native_socket_t const       _sock;
    bool                        _close_on_destruction;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_SOCKET_HPP_INCLUDED_2008_04_20
