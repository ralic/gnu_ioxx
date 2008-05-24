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

#ifndef IOXX_ACCEPTOR_HPP_INCLUDED_2008_05_19
#define IOXX_ACCEPTOR_HPP_INCLUDED_2008_05_19

#include <ioxx/dispatch.hpp>
#include <boost/function/function2.hpp>

namespace ioxx
{
  /**
   * Accept incoming stream connections on a local network port. An acceptor is
   * given a socket::endpoint and a handler function. Whenever a new connection
   * is received on that particular endpoint, the acceptor calls the handler
   * function with the newly received socket::native_t and the socket::address
   * of the peer. It's the handler functions responsibility to do something
   * with the socket, i.e. to register it in an event dispatcher. If the
   * handler function throws an exception, however, the newly received socket
   * is closed before the exception is propagated.
   *
   * \sa \ref inetd
   */
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
    typedef Handler                     handler;

    /**
     * Create an acceptor object.
     *
     * \param disp The i/o event dispatcher (i.e. core) to register this acceptor in.
     * \param addr Create a listening socket that's bound to this particular endpoint.
     * \param f    Callback function to invoke every time new connection is received.
     */
    acceptor(dispatch & disp, endpoint const & addr, handler const & f = handler())
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

#endif // IOXX_ACCEPTOR_HPP_INCLUDED_2008_05_19
