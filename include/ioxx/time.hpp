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

#ifndef IOXX_TIME_HPP_INCLUDED_2008_05_19
#define IOXX_TIME_HPP_INCLUDED_2008_05_19

#include <ioxx/error.hpp>
#include <boost/noncopyable.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <sys/time.h>

namespace ioxx
{
  /**
   * The current time of day, represented in seconds since 1970-01-01 00:00:00
   * UTC.
   */
  typedef std::time_t time_t;

  /**
   * An (unsigned) quantity of seconds.
   *
   * This type is useful for specifying time durations, i.e. timeouts that will
   * occur at some point in the future.
   */
  typedef unsigned int seconds_t;

  /**
   * The current time of day in microseconds since 1970-01-01 00:00:00 UTC.
   *
   * POSIX.1-2001 defines this type to contain (at least) the following
   * members:
   *
   * \code
   *   struct timeval
   *   {
   *     time_t      tv_sec;     // seconds
   *     suseconds_t tv_usec;    // microseconds
   *   };
   * \endcode
   *
   * The type \c suseconds_t is usually signed for history reasons (it used to
   * be a \c long).
   */
  typedef ::timeval timeval;

  /**
   * Access to the current time of day.
   */
  class time_of_day : private boost::noncopyable
  {
  public:
    /**
     * A default-constructed object has the accurate time of day.
     */
    time_of_day() { update(); }

    /**
     * Return the current time of day.
     */
    time_t const &  as_time_t() const   { return _now.tv_sec; }

    /**
     * Return the current time of day.
     */
    timeval const & as_timeval() const  { return _now; }

    /**
     * Update the time of day.
     */
    void update()
    {
      throw_errno_if_minus1("gettimeofday(2)", boost::bind(boost::type<int>(), gettimeofday, &_now, static_cast<struct timezone *>(0)));
    }

  private:
    timeval _now;
  };

} // namespace ioxx

#endif // IOXX_TIME_HPP_INCLUDED_2008_05_19
