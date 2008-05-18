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

#ifndef IOXX_ERROR_HPP_INCLUDED_2008_04_20
#define IOXX_ERROR_HPP_INCLUDED_2008_04_20

#include <stdexcept>
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <functional>

namespace ioxx
{
  /**
   * System errors are nondescript and sudden.
   */
  struct system_error : public std::runtime_error
  {
    int error_code;

    explicit system_error(int ec, std::string const & context)
    : std::runtime_error(std::string(context + ": " + std::strerror(ec)))
    , error_code(ec)
    {
    }
  };

  /**
   * Safety-wrapper for calling POSIX system functions.
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

  template <class Predicate, class Action>
  inline typename Action::result_type throw_errno_if(Predicate const & is_failure, std::string const & error_msg, Action const & f)
  {
    return throw_errno_if<typename Action::result_type>(is_failure, error_msg, f);
  }

  template <class Num, class Action>
  inline Num throw_errno_if_minus1(std::string const & error_msg, Action const & f)
  {
    return throw_errno_if<Num>(boost::bind<Num>(std::equal_to<Num>(), static_cast<Num>(-1), _1), error_msg, f);
  }

  template <class Action>
  inline typename Action::result_type throw_errno_if_minus1(std::string const & error_msg, Action const & f)
  {
    typedef typename Action::result_type Num;
    return throw_errno_if<Num>(boost::bind<Num>(std::equal_to<Num>(), static_cast<Num>(-1), _1), error_msg, f);
  }

  struct not_ewould_block : public std::unary_function<ssize_t, bool>
  {
    bool operator() (ssize_t rc) const
    {
      return rc < 0 && errno != EWOULDBLOCK && errno != EAGAIN;
    }
  };

} // namespace ioxx

#endif // IOXX_ERROR_HPP_INCLUDED_2008_04_20
