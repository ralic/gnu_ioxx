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
  class core : public dispatch<>
  {
  public:
    typedef schedule<>                  schedule;
    typedef schedule::timeout           timeout;
    typedef dispatch<>                  dispatch;
    typedef boost::function0<void>      handler;

    class socket : public dispatch::socket
    {
    public:
      socket(core & io, native_socket_t sock)
      : dispatch::socket( io, sock, boost::bind(&socket::run, this, _1), dispatch::socket::no_events)
      , _to_in(io._schedule), _to_out(io._schedule), _to_pri(io._schedule)
      {
      }

      void on_input(handler const & f)
      {
        set(_in, _to_in, f, 0u, handler());
      }

      void on_input(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_in, _to_in, f, tout, on_timeout);
      }

      void on_output(handler const & f)
      {
        set(_out, _to_out, f, 0u, handler());
      }

      void on_output(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_out, _to_out, f, tout, on_timeout);
      }

      void on_priput(handler const & f)
      {
        set(_pri, _to_pri, f, 0u, handler());
      }

      void on_priput(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_pri, _to_pri, f, tout, on_timeout);
      }

    protected:
      core & context() { return static_cast<core &>(dispatch::socket::context()); }

    private:
      handler _in, _out, _pri;
      timeout _to_in, _to_out, _to_pri;

      void set(handler & f_, timeout & to_f_, handler f, seconds_t tout, handler const & on_timeout)
      {
        BOOST_ASSERT(tout == 0u || on_timeout);
        timeout to( context()._schedule );
        if (tout && on_timeout) to.reset(context()._now.as_time_t() + tout, on_timeout);
        to.swap(to_f_);
        f.swap(f_);
        if (f_ && !f)
        {
          try { register_events(); }
          catch(...)
          {
            to.swap(to_f_);
            f.swap(f_);
            throw;
          }
        }
      }

      void register_events()
      {
        dispatch::socket::event_set req( dispatch::socket::no_events );
        if (_in)  req |= dispatch::socket::readable;
        if (_out) req |= dispatch::socket::writable;
        if (_pri) req |= dispatch::socket::pridata;
        this->request(req);
      }

      void run(dispatch::socket::event_set ev)
      {
        if (ev & dispatch::socket::readable && _in)
        {
          handler f;
          _to_in.reset();
          f.swap(_in);
          f();
        }
        if (ev & dispatch::socket::writable && _out)
        {
          handler f;
          _to_out.reset();
          f.swap(_out);
          f();
        }
        if (ev & dispatch::socket::pridata && _pri)
        {
          handler f;
          _to_pri.reset();
          f.swap(_pri);
          f();
        }
        if (ev != dispatch::socket::no_events)
          register_events();
      }
    };

    void run()
    {
      for (;;)
      {
        dispatch::run();
        seconds_t to( _schedule.run(_now.as_time_t()) );
        if (_schedule.empty())
        {
          if (dispatch::empty()) break;
          else                   to = dispatch::max_timeout();
        }
        dispatch::wait(to);
        _now.update();
      }
    }

  private:
    time        _now;
    schedule    _schedule;
  };

} // namespace ioxx

#endif // IOXX_CORE_HPP_INCLUDED_2008_04_20
