#ifndef IOXX_TIMER_HPP_INCLUDED_2008_04_20
#define IOXX_TIMER_HPP_INCLUDED_2008_04_20

#include "error.hpp"
#include <boost/noncopyable.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <sys/time.h>

namespace ioxx
{
  using std::time_t;

  class timer : private boost::noncopyable
  {
  public:
    timer()                             { update(); }

    time_t const &  as_time_t() const   { return _now.tv_sec; }
    timeval const & as_timeval() const  { return _now; }

    void update()
    {
      throw_errno_if_minus1_<int>( "system call gettimeofday(2) failed"
                                 , boost::bind(gettimeofday, &_now, static_cast<struct timezone *>(0))
                                 );
    }

  private:
    timeval _now;
  };

} // namespace ioxx

#endif // IOXX_TIMER_HPP_INCLUDED_2008_04_20
