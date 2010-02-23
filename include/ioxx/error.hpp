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

#ifndef IOXX_ERROR_HPP_INCLUDED_2010_02_23
#define IOXX_ERROR_HPP_INCLUDED_2010_02_23

#include <stdexcept>
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <cstring>

namespace ioxx
{
  /**
   * Base exception class for all operating system errors. System errors are
   * are caused by external conditions such as a network connection timing out,
   * lack of resources, or some other OS-specific problem. The exact nature of
   * the error is described by the standard \c errno code, respectively a
   * human-readable string that can be obtained through the inherited method
   * std::exception::what(). For example, the expression
   *
   * \verbatim system_error(ENOSPC, "writing to /tmp/test.data").what() \endverbatim
   *
   * would return:
   *
   * \verbatim writing to /tmp/test.data: No space left on device \endverbatim
   *
   * The textual descriptions are obtained from std::strerror().
   *
   * \sa throw_errno_if
   */
  class system_error : public std::runtime_error
  {
  public:
    /**
     * Construct a system_error using an \c errno and a textual context
     * description. The string used to describe the context will be prepended
     * to the actual error message that can be retrieved with
     * std::exception::what().
     *
     * \param ec        Native \c errno code that describes the error.
     * \param context   Description of the system call context that failed.
     */
    system_error(int ec, std::string const & context)
    : std::runtime_error(std::string(context + ": " + std::strerror(ec)))
    , error_code(ec)
    {
    }

    /// The \c errno value returned by operating system.
    int error_code;
  };

  /**
   * Safety-wrapper for calling POSIX system functions. Pretty much all system
   * functions may fail with the \c EINTR condition, meaning that the call was
   * interrupted by a signal. The throw_errno_if() combinator handles this
   * problem transparently and restarts the function if that condition arises.
   * All other errors will be signaled by means of a system_error exception.
   *
   * \param is_failure  Predicate of type <code>bool (Result)</code> that
   *                    returns \c true if the returned value constitutes an
   *                    error.
   * \param error_msg   Context string to be used when constructing a
   *                    system_error in case of an error.
   * \param f           Functor of type <code>Result ()</code> that performs
   *                    the desired system call.
   * \return            The return value returned by \c f.
   * \throw system_error Thrown if the \c is_failure predicate returns \c true.
   *
   * \sa signal_block
   */
  template <class Result, class Predicate, class Action>
  inline Result throw_errno_if(Predicate is_failure, std::string const & error_msg, Action f)
  {
    unsigned int max_retries( 5u );
    Result r;
    for(r = f(); is_failure(r); r = f())
    {
      if (errno == EINTR && max_retries--) continue;
      system_error err(errno, error_msg);
      throw err;
    }
    BOOST_ASSERT(!is_failure(r));
    return r;
  }

  /**
   * Overloaded variant that automatically deduces the return type.
   */
  template <class Predicate, class Action>
  inline typename Action::result_type throw_errno_if(Predicate const & is_failure, std::string const & error_msg, Action const & f)
  {
    return throw_errno_if<typename Action::result_type>(is_failure, error_msg, f);
  }

  /**
   * Specialized version of throw_errno_if() for system calls that return -1 on
   * error. This combinator removes the need to specify an \c is_failure
   * predicate.
   *
   * \param error_msg   Context string to be used when constructing a
   *                    system_error in case of an error.
   * \param f           Functor of type <code>Num ()</code> that performs
   *                    the desired system call.
   * \return            The return value returned by \c f.
   * \throw system_error Thrown if the \c is_failure predicate returns \c true.
   */
  template <class Num, class Action>
  inline Num throw_errno_if_minus1(std::string const & error_msg, Action const & f)
  {
    return throw_errno_if<Num>(boost::bind<Num>(std::equal_to<Num>(), static_cast<Num>(-1), _1), error_msg, f);
  }

  /**
   * Overloaded variant that automatically deduces the return type.
   */
  template <class Action>
  inline typename Action::result_type throw_errno_if_minus1(std::string const & error_msg, Action const & f)
  {
    typedef typename Action::result_type Num;
    return throw_errno_if<Num>(boost::bind<Num>(std::equal_to<Num>(), static_cast<Num>(-1), _1), error_msg, f);
  }

} // namespace ioxx

#endif // IOXX_ERROR_HPP_INCLUDED_2010_02_23
