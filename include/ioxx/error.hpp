#ifndef IOXX_ERROR_HPP_INCLUDED_2008_04_20
#define IOXX_ERROR_HPP_INCLUDED_2008_04_20

#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>
#include <functional>

namespace ioxx
{
  template <class Result, class Predicate, class Action>
  inline Result throw_errno_if(Predicate is_failure, std::string const & error_msg, Action f)
  {
    unsigned int max_retries( 5u );
    Result r;
    for(r = f(); is_failure(r); r = f())
    {
      if (errno == EINTR && --max_retries > 0u) continue;
      boost::system::system_error err(errno, boost::system::errno_ecat, error_msg);
      throw err;
    }
    BOOST_ASSERT(!is_failure(r));
    return r;
  }

  template <class Result, class Predicate, class Action>
  inline void throw_errno_if_(Predicate const & is_failure, std::string const & error_msg, Action const & f)
  {
    throw_errno_if<Result>(is_failure, error_msg, f);
  }

  template <class Num, class Action>
  inline Num throw_errno_if_minus1(std::string const & error_msg, Action const & f)
  {
    return throw_errno_if<Num>(boost::bind(std::equal_to<Num>(), static_cast<Num>(-1), _1), error_msg, f);
  }

  template <class Num, class Action>
  inline void throw_errno_if_minus1_(std::string const & error_msg, Action const & f)
  {
    throw_errno_if_minus1<Num>(error_msg, f);
  }

} // namespace ioxx

#endif // IOXX_ERROR_HPP_INCLUDED_2008_04_20
