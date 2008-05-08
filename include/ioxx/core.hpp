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
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
#  include <ioxx/detail/adns.hpp>
#else
#  error "No asynchronous DNS resolver available on this platform."
#endif

namespace ioxx
{
  template < class Allocator = std::allocator<void> >
  class core : public time
             , public schedule<Allocator>
             , public dispatch<Allocator>
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
             , public detail::adns<Allocator>
#endif
  {
  public:
    typedef Allocator                           allocator;
    typedef schedule<allocator>                 schedule;
    typedef typename schedule::timeout          timeout;
    typedef dispatch<allocator>                 dispatch;
    typedef typename dispatch::socket           socket;
    typedef detail::adns<allocator>             dns;

    core() : schedule(time::as_time_t()), dns(*this, *this, time::as_timeval())
    {
    }

    seconds_t run()
    {
      dispatch::run();
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
      time::update();
    }
  };

} // namespace ioxx

#endif // IOXX_CORE_HPP_INCLUDED_2008_04_20
