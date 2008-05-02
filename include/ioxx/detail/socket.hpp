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

#include <ioxx/detail/config.hpp>
#include <ioxx/detail/error.hpp>
#include <boost/noncopyable.hpp>
#include <iosfwd>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <boost/function/function2.hpp>

namespace ioxx { namespace detail
{
  class socket : private boost::noncopyable
  {
  public:
    enum service_type_tag { stream_service = SOCK_STREAM, datagram_service = SOCK_DGRAM };

    class address
    {
    public:
      typedef char host_name[NI_MAXHOST];
      typedef char service_name[NI_MAXSERV];

      address() : _len(0) { }
      address(sockaddr const & addr, socklen_t len) : _addr(addr), _len(len) { }

      address(char const * host, char const * service, service_type_tag type = stream_service)
      {
        addrinfo const hint = { AI_NUMERICHOST, 0, type, 0, 0u, NULL, NULL, NULL };
        addrinfo * addr;
        int const rc( getaddrinfo(host, service, &hint, &addr) );
        if (rc != 0) throw std::runtime_error( gai_strerror(rc) );
        BOOST_ASSERT(addr);
        BOOST_ASSERT(addr->ai_socktype == type);        // we got the type we requested
        BOOST_ASSERT(!addr->ai_next);                   // only one result
        BOOST_ASSERT(!addr->ai_addrlen <= sizeof(sockaddr));
        _addr = *addr->ai_addr;
        _len  = addr->ai_addrlen;
        freeaddrinfo(addr);
      }

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

      sockaddr &         as_sockaddr()        { return _addr; }
      sockaddr const &   as_sockaddr()  const { return _addr; }

      socklen_t &        as_socklen_t()       { return _len; }
      socklen_t const &  as_socklen_t() const { return _len; }

      friend std::ostream & operator<< (std::ostream & os, address const & addr) { return os << addr.show(); }

    protected:
      sockaddr  _addr;
      socklen_t _len;
    };

    class endpoint : public address
    {
    public:
      endpoint() { }

      endpoint(char const * host, char const * service, service_type_tag type = stream_service)
      {
        addrinfo const hint = { AI_NUMERICHOST, 0, type, 0, 0u, NULL, NULL, NULL };
        addrinfo * addr;
        int const rc( getaddrinfo(host, service, &hint, &addr) );
        if (rc != 0) throw std::runtime_error( gai_strerror(rc) );
        BOOST_ASSERT(addr);
        BOOST_ASSERT(addr->ai_socktype == type);        // we got the type we requested
        BOOST_ASSERT(!addr->ai_next);                   // only one result
        BOOST_ASSERT(!addr->ai_addrlen <= sizeof(sockaddr));
        _addr     = *addr->ai_addr;
        _len      = addr->ai_addrlen;
        _domain   = addr->ai_family;
        _socktype = addr->ai_socktype;
        _protocol = addr->ai_protocol;
        freeaddrinfo(addr);
      }

      int &         as_domain()        { return _domain; }
      int const &   as_domain()  const { return _domain; }

      int &         as_socktype()        { return _socktype; }
      int const &   as_socktype()  const { return _socktype; }

      int &         as_protocol()        { return _protocol; }
      int const &   as_protocol()  const { return _protocol; }

    protected:
      int       _domain;
      int       _socktype;
      int       _protocol;
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

    static native_socket_t create(endpoint const & addr)
    {
      return throw_errno_if_minus1("socket(2)", boost::bind(&::socket, addr.as_domain(), addr.as_socktype(), addr.as_protocol()));
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

    void set_linger_timeout(unsigned short second_timeout)
    {
      linger ling;
      ling.l_onoff  = second_timeout > 0 ? 1 : 0;
      ling.l_linger = static_cast<int>(second_timeout);
      throw_errno_if_minus1("set socket lingering", boost::bind(&::setsockopt, _sock, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)));
    }

    void reuse_bind_address(bool enable = true)
    {
      int true_flag = enable ? 1 : 0;
      throw_errno_if_minus1("bind with SO_REUSEADDR", boost::bind(&::setsockopt, _sock, SOL_SOCKET, SO_REUSEADDR, &true_flag, sizeof(int)));
    }

    void bind(address const & addr)
    {
      throw_errno_if_minus1("bind(2)", boost::bind(&::bind, _sock, &addr.as_sockaddr(), addr.as_socklen_t()));
    }

    void listen(unsigned short backlog)
    {
      throw_errno_if_minus1("listen(2)", boost::bind(&::listen, _sock, static_cast<int>(backlog)));
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

    ssize_t readv(iovec * begin, iovec const * end)
    {
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "readv(2)"
                                      , boost::bind(&::readv, _sock, begin, static_cast<int>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "readv(2) received " << rc << " bytes from " << this);
      return rc;
    }

    ssize_t writev(iovec const * begin, iovec const * end)
    {
      BOOST_ASSERT(begin < end);
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "writev(2)"
                                      , boost::bind(&::writev, _sock, begin, static_cast<int>(end - begin))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "writev(2) wrote " << rc << " bytes to " << this);
      return rc;
    }

    ssize_t recv_from(iovec * begin, iovec const * end, address & from)
    {
      BOOST_ASSERT(begin < end);
      msghdr msg =
        { &from.as_sockaddr()
        , static_cast<socklen_t>(sizeof(sockaddr))
        , begin
        , static_cast<size_t>(end - begin)
        , static_cast<void *>(0)                    // control data
        , static_cast<socklen_t>(0)                 // control data size
        , static_cast<int>(0)                       // flags: set on return
        };
      ssize_t const rc( throw_errno_if( not_ewould_block()
                                      , "recvmsg(2)"
                                      , boost::bind(&::recvmsg, _sock, &msg, static_cast<int>(MSG_DONTWAIT))
                                      ));
      IOXX_TRACE_SOCKET(_sock, "recvmsg(2) received " << rc << " bytes from " << this);
      from.as_socklen_t() = msg.msg_namelen;
      return rc;
    }

    ssize_t send_to(iovec const * begin, iovec const * end, address const & to)
    {
      BOOST_ASSERT(begin < end);
      msghdr msg =
        { const_cast<sockaddr *>(&to.as_sockaddr())
        , to.as_socklen_t()
        , const_cast<iovec *>(begin)
        , static_cast<size_t>(end - begin)
        , static_cast<void *>(0)                    // control data
        , static_cast<socklen_t>(0)                 // control data size
        , static_cast<int>(0)                       // flags: set on return
        };
      return throw_errno_if(not_ewould_block(), "sendmsg(2)", boost::bind(&::sendmsg, _sock, &msg, static_cast<int>(MSG_DONTWAIT)));
    }

    address local_address() const
    {
      address addr(sockaddr(), sizeof(sockaddr));
      throw_errno_if_minus1("getsockname(2)", boost::bind(&::getsockname, _sock, &addr.as_sockaddr(), &addr.as_socklen_t()));
      return addr;
    }

    address peer_address() const
    {
      address addr(sockaddr(), sizeof(sockaddr));
      throw_errno_if_minus1("getpeername(2)", boost::bind(&::getpeername, _sock, &addr.as_sockaddr(), &addr.as_socklen_t()));
      return addr;
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
