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
    : socket(disp, addr.create(), boost::bind(&acceptor::run, this), socket::readable)
    , _f(f)
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.acceptor." + addr.show());
      this->set_nonblocking();
      this->reuse_bind_address();
      this->bind(addr);
      this->listen(16u);
      IOXX_TRACE_MSG("accepting connections on " << addr);
    }

  protected:
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);

  private:
    handler     _f;

    void run()
    {
      native_socket_t s;
      address addr;
      while(this->accept(s, addr))
      {
        detail::socket new_socket(s); // act as scope guard
        IOXX_TRACE_MSG("accepted connection from " << addr << " on " << s);
        new_socket.set_nonblocking();
        new_socket.set_linger_timeout(0);
        _f(s, addr);
        new_socket.close_on_destruction(false);
      }
    }
  };

} // namespace ioxx

#endif // IOXX_ACCEPTOR_HPP_INCLUDED_2008_04_20
