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
  template < class Allocator = std::allocator<void>
           , class Dispatch  = dispatch<Allocator>
           , class Handler   = boost::function2< void
                                               , typename Dispatch::socket::native_t
                                               , typename Dispatch::socket::address const &
                                               , typename Allocator::template rebind<boost::function_base>::other
                                               >
           >
  class acceptor : private boost::noncopyable
  {
  public:
    typedef Dispatch                    dispatch;
    typedef typename dispatch::socket   socket;
    typedef typename socket::address    address;
    typedef typename socket::endpoint   endpoint;
    typedef typename socket::event_set  event_set;
    typedef Handler                     handler;

    acceptor(dispatch & disp, endpoint const & addr, handler const & f)
    : _ls(disp, addr.create(), boost::bind(&acceptor::run, this), socket::readable)
    , _f(f)
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.acceptor." + detail::show(_ls.as_native_socket_t()));
      _ls.set_nonblocking();
      _ls.reuse_bind_address();
      _ls.bind(addr);
      _ls.listen(16u);
      LOGXX_TRACE("accepting connections on " << addr);
    }

  protected:
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);

  private:
    socket      _ls;
    handler     _f;

    void run()
    {
      native_socket_t s;
      address addr;
      while(_ls.accept(s, addr))
      {
        system_socket new_socket(s); // act as scope guard
        LOGXX_TRACE("accepted connection from " << addr << " on " << new_socket);
        new_socket.set_nonblocking();
        new_socket.set_linger_timeout(0);
        _f(s, addr);
        new_socket.close_on_destruction(false);
      }
    }
  };

} // namespace ioxx

#endif // IOXX_ACCEPTOR_HPP_INCLUDED_2008_04_20
