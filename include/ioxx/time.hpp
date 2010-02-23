/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_TIME_HPP_INCLUDED_2010_02_23
#define IOXX_TIME_HPP_INCLUDED_2010_02_23

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
   * \internal
   *
   * \brief Access to the current time of day.
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
    time_t const &  current_time_t() const   { return _now.tv_sec; }

    /**
     * Return the current time of day.
     */
    timeval const & current_timeval() const  { return _now; }

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

#endif // IOXX_TIME_HPP_INCLUDED_2010_02_23
