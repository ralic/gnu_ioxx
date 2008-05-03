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

#ifndef IOXX_CORE_HPP_INCLUDED_2008_04_20
#define IOXX_CORE_HPP_INCLUDED_2008_04_20

#include <ioxx/time.hpp>
#include <ioxx/schedule.hpp>
#include <ioxx/dispatch.hpp>
#include <ioxx/dns.hpp>
#include <ioxx/acceptor.hpp>
#include <boost/shared_ptr.hpp>

namespace ioxx
{
  class core : protected dispatch<>
  {
  public:
    typedef schedule<>                  schedule;
    typedef dispatch<>                  dispatch;
    typedef boost::function0<void>      handler;

    class socket : public dispatch::socket
    {
    public:
      socket(core & io, native_socket_t sock)
      : dispatch::socket( _dispatch, sock, boost::bind(&socket::run, this, _1), dispatch::socket::no_events)
      {
      }

    protected:
      core & context() { return static_cast<core &>(dispatch::socket::context()); }

    private:
      handler _in, _out, _pri;

      void run(dispatch::socket::event_set ev)
      {
      }
    };

  private:
    time        _now;
    schedule    _schedule;
    dispatch    _dispatch;
  };

} // namespace ioxx

#endif // IOXX_CORE_HPP_INCLUDED_2008_04_20
