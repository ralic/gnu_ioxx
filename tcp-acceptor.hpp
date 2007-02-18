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

#ifndef IOXX_TCP_ACCEPTOR_HPP_INCLUDED
#define IOXX_TCP_ACCEPTOR_HPP_INCLUDED

#include "probe.hpp"
#include "logging.hpp"

namespace ioxx
{
  /**
   *  \brief todo
   */
  template <class AcceptorT>
  struct tcp_acceptor : public AcceptorT
  {
    tcp_acceptor()                                      { }
    tcp_acceptor(AcceptorT const & f) : AcceptorT(f)    { }

    event operator() (socket const & ls_, event ev, probe & probe) const
    {
      listen_socket ls( ls_ );
      if (no_error(ev) && is_readable(ev))
      {
        I(ls);
        try
        {
          for(;;)
          {
            data_socket s( ls.accept() );
            if (!s) return Readable;
            IOXX_INFO_MSG(catchall) << "socket " << s << ": start new handler ";
            AcceptorT::operator()(s, probe);
          }
        }
        catch(std::exception const & e)
        {
          IOXX_ERROR_MSG(catchall) << "caught exception: " << e.what();
        }
        catch(...)         /// \todo Give handler a chance to recover.
        {
          IOXX_ERROR_MSG(catchall) << "caught unknown exception";
        }
      }
      return Error;
    }
  };

  /**
   *  \brief todo
   */
  template <class AcceptorT>
  inline listen_socket add_tcp_acceptor
    ( probe &           probe
    , AcceptorT const & f
    , tcp_port_t        port
    , ipv4_address_t    addr    = ipv4_address_t(0u)
    , unsigned int      quelen  = 32u
    )
  {
    listen_socket ls(port, addr, quelen);
    I(ls);
    probe.add(ls, Readable, tcp_acceptor<AcceptorT>(f));
    return ls;
  }

  /**
   *  \brief todo
   */
  template <class HandlerT>
  inline listen_socket add_tcp_service
    ( probe &           probe
    , tcp_port_t        port
    , ipv4_address_t    addr    = ipv4_address_t(0u)
    , unsigned int      quelen  = 32u
    )
  {
    return add_tcp_acceptor(probe, typename HandlerT::acceptor(), port, addr, quelen);
  }

} // namespace ioxx

#endif // IOXX_TCP_ACCEPTOR_HPP_INCLUDED
