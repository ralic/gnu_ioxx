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

#ifndef IOXX_ACCEPTOR_HPP_INCLUDED_2008_04_20
#define IOXX_ACCEPTOR_HPP_INCLUDED_2008_04_20

#include <ioxx/dispatch.hpp>
#include <boost/function/function2.hpp>

namespace ioxx
{
  template < class Dispatch = dispatch<>
           , class Handler  = boost::function2<void, native_socket_t, typename Dispatch::socket::address const &>
           >
  class acceptor : public Dispatch::socket
  {
  public:
    typedef Dispatch                    dispatch;
    typedef typename dispatch::socket   socket;
    typedef typename socket::address    address;
    typedef typename socket::endpoint   endpoint;
    typedef typename socket::event_set  event_set;
    typedef Handler                     handler;

    acceptor(dispatch & disp, endpoint const & addr, handler const & f = handler())
    : socket(disp, socket::create(addr), boost::bind(&acceptor::run, this), socket::readable)
    , _f(f)
    {
      this->set_nonblocking();
      this->reuse_bind_address();
      this->bind(addr);
      this->listen(16u);
    }

  private:
    handler     _f;

    void run()
    {
      native_socket_t s;
      address addr;
      for(;;)
      {
        addr.as_socklen_t() = sizeof(sockaddr);
        s = detail::throw_errno_if( detail::not_ewould_block(), "accept(2)"
                                  , boost::bind(&::accept, this->as_native_socket_t(), &addr.as_sockaddr(), &addr.as_socklen_t())
                                  );
        if (s < 0) break;
        detail::socket new_socket(s); // act as scope guard
        new_socket.set_nonblocking();
        new_socket.set_linger_timeout(0);
        _f(s, addr);
        new_socket.close_on_destruction(false);
      }
    }
  };

} // namespace ioxx

#endif // IOXX_ACCEPTOR_HPP_INCLUDED_2008_04_20
