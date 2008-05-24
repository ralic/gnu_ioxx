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

#ifndef IOXX_CORE_HPP_INCLUDED_2008_05_19
#define IOXX_CORE_HPP_INCLUDED_2008_05_19

#include <ioxx/time.hpp>
#include <ioxx/schedule.hpp>
#include <ioxx/dispatch.hpp>
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
#  include <ioxx/detail/adns.hpp>
#else
#  error "No asynchronous DNS resolver available on this platform."
#endif

namespace ioxx
{
  /**
   * Asynchronous interface to socket I/O, time events, and DNS.
   *
   * \sa \ref inetd
   */
  template < class Allocator = std::allocator<void> >
  class core : public time_of_day
             , public dispatch<Allocator>
             , public schedule<Allocator>
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
             , public detail::adns<Allocator>
#endif
  {
  public:
    typedef Allocator                           allocator;
    typedef schedule<allocator>                 schedule;
    typedef dispatch<allocator>                 dispatch;
    typedef detail::adns<allocator>             dns;

    /**
     * An event-driven socket.
     *
     */
    class socket : public dispatch::socket
    {
    public:
      typedef typename dispatch::socket::event_set event_set;
      typedef typename dispatch::socket::handler   handler;

      /**
       * Register a socket in the i/o event dispatcher.
       *
       * \param io   The core object to register this socket in.
       * \param sock The native socket.
       * \param ev   Event set to wait for.
       * \param f    Callback function to invoke when an event occurs.
       */
      socket(core & io, native_socket_t sock, handler const & f = handler(), event_set ev = socket::no_events)
      : dispatch::socket(io, sock, f, ev)
      {
      }

      core &         get_core()         { return context(); }
      core const &   get_core() const   { return context(); }

    protected:
      core & context() { return static_cast<core &>(dispatch::socket::context()); }
    };

    class timeout : public schedule::timeout
    {
    public:
      typedef typename schedule::task task;

      timeout(core & sched) : schedule::timeout(sched)
      {
      }

      timeout(core & sched, time_t ts, task const & f) : schedule::timeout(sched, ts, f)
      {
      }

      timeout(core & sched, seconds_t to, task const & f) : schedule::timeout(sched, to, f)
      {
      }

      core &        get_core()          { return static_cast<core &>(schedule::timeout::get_schedule()); }
      core const &  get_core() const    { return static_cast<core const &>(schedule::timeout::get_schedule()); }
    };

    core() : schedule(time_of_day::current_time_t()), dns(*this, *this, time_of_day::current_timeval())
    {
    }

    bool empty() const
    {
      return schedule::empty() && dispatch::empty() && dns::empty();
    }

    seconds_t run()
    {
      dispatch::run();
      schedule::run();
      dns::run();
      seconds_t timeout( schedule::run() );
      if (schedule::empty())
      {
        if (dispatch::empty())  BOOST_ASSERT(timeout == 0u);
        else                    timeout = dispatch::max_timeout();
      }
      return timeout;
    }

    void wait(seconds_t timeout)
    {
      dispatch::wait(timeout);
      time_of_day::update();
    }
  };

} // namespace ioxx

#endif // IOXX_CORE_HPP_INCLUDED_2008_05_19
