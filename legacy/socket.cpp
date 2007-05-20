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

#include <sys/types.h>
#include <unistd.h>             // close(2)
#include <fcntl.h>              // fcntl(2)
#include <sys/socket.h>         // setsockopt(2)
#include <netinet/in.h>         // sockaddr_in
#include <cerrno>
#include "sanity/system-error.hpp"
#include "ioxx/socket.hpp"

// ----- Utility Functions ----------------------------------------------------

static ioxx::socket make_tcp_socket()
{
  int const fd( ::socket(AF_INET, SOCK_STREAM, 0) );
  if (fd < 0) throw system_error("cannot create TCP socket");
  return ioxx::posix_socket(fd);
}

static void disable_linger_mode(ioxx::socket const & s)
{
  I(s);
  ::linger ling;
  ling.l_onoff  = 0;
  ling.l_linger = 0;
  if (::setsockopt(s->fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
    throw system_error("cannot disable TCP lingering mode");
}

static void enable_tcp_reuse(ioxx::socket const & s)
{
  I(s);
  int true_flag = 1;
  if (::setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &true_flag, sizeof(int)) == -1)
    throw system_error("cannot enable TCP port re-use");
}

static void bind_tcp_socket( ioxx::socket const &  s
                           , ioxx::tcp_port_t      port
                           , ioxx::ipv4_address_t  addr
                           )
{
  I(s);
  sockaddr_in  sin;
  socklen_t    sin_size;
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = htonl(addr);
  sin.sin_port        = htons(port);
  sin_size            = sizeof(sin);
  if (::bind(s->fd, (sockaddr*)&sin, sin_size) == -1)
    throw system_error("cannot bind TCP socket");
}

static void make_listen_socket( ioxx::socket const &    s
                              , unsigned int            queue_backlog
                              )
{
  I(s);
  if (::listen(s->fd, queue_backlog) == -1)
    throw system_error("cannot listen on TCP");
}

// ----- System Socket --------------------------------------------------------

ioxx::socket ioxx::posix_socket(int fd)
{
  socket s;
  if (fd >= 0)
  {
    s.reset(new system_socket(fd));
    if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
      throw system_error("socket does not support non-blocking operations");
  }
  return s;
}

ioxx::system_socket::~system_socket() SANITY_NOTHROW_DECL
{
  for(unsigned int retries(3u); retries > 0; --retries)
  {
    int const rc( ::close(fd) );
    if (rc == 0 || errno != EINTR)
      break;
  }
}

// ----- Listen Socket --------------------------------------------------------

ioxx::listen_socket::listen_socket( tcp_port_t     port
                                  , ipv4_address_t addr
                                  , unsigned int   quelen
                                  )
{
  socket::operator=( make_tcp_socket() );
  enable_tcp_reuse(*this);
  bind_tcp_socket(*this, port, addr);
  make_listen_socket(*this, quelen);
}

ioxx::socket ioxx::listen_socket::accept()
{
  int const fd( ::accept((*this)->fd, NULL, 0) );
  if (fd <= 0)
  {
    if (errno == EAGAIN)  return socket();
    else                  throw system_error("cannot accept new connection");
  }
  else
  {
    socket s( ioxx::posix_socket(fd) );
    disable_linger_mode(s);
    return s;
  }
}

// ----- Data Socket ----------------------------------------------------------

char * ioxx::data_socket::read(char * begin, char const * end)
{
  I(*this); I(begin < end);
  size_t const len( end - begin );
  int const rc( ::read((*this)->fd, begin, len) );
  if      (rc >  0)           return begin + rc;
  else if (rc == 0)           return NULL;
  else if (errno == EAGAIN)   return begin;
  else                        throw system_error("read() failed");
}

char const * ioxx::data_socket::write(char const * begin, char const * end)
{
  I(*this); I(begin < end);
  size_t const len( end - begin );
  int const rc( ::write((*this)->fd, begin, len) );
  if      (rc > 0)           return begin + rc;
  else if (rc == 0)          return NULL;
  else if (errno == EAGAIN)  return begin;
  else                       throw system_error("write() failed");
}

char * ioxx::data_socket::write(char * begin, char const * end)
{
  return const_cast<char *>(write(const_cast<char const *>(begin), end));
}
