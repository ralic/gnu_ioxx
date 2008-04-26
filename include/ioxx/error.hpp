#ifndef IOXX_ERROR_HPP_INCLUDED_2008_04_20
#define IOXX_ERROR_HPP_INCLUDED_2008_04_20

#include <boost/system/system_error.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <functional>

#ifndef NDEBUG
#  include <iostream>
#  define IOXX_TRACE_MSG(msg) std::cout << msg << std::endl
#else
#  define IOXX_TRACE_MSG(msg) static_cast<void>(0)
#endif
#define IOXX_TRACE_SOCKET(s,msg) IOXX_TRACE_MSG("socket " << s << ": " << msg)

namespace ioxx
{
  template <class Result, class Predicate, class Action>
  inline Result throw_errno_if(Predicate is_failure, std::string const & error_msg, Action f)
  {
    unsigned int max_retries( 5u );
    Result r;
    for(r = f(); is_failure(r); r = f())
    {
      if (errno == EINTR && max_retries--) continue;
      boost::system::system_error err(errno, boost::system::errno_ecat, error_msg);
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

} // namespace ioxx

#endif // IOXX_ERROR_HPP_INCLUDED_2008_04_20
