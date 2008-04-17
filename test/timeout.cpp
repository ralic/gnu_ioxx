#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/strong_typedef.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/system/system_error.hpp>
#include <map>

namespace ioxx
{
  BOOST_STRONG_TYPEDEF(std::time_t, time_t);
  BOOST_STRONG_TYPEDEF(unsigned int, second_t);
  BOOST_STRONG_TYPEDEF(unsigned int, nanosecond_t);

  class system_time : private boost::noncopyable
  {
  public:
    system_time()                       { update(); }

    static time_t const &  now()        { return _now; }
    static void            update();

  private:
    static time_t _now;
  };

  time_t system_time::_now = ioxx::time_t(0);

  void ioxx::system_time::update()
  {
    using namespace boost::system;
    std::time_t new_now;
    if (time(&new_now) == std::time_t(-1))
      throw system_error(errno, errno_ecat, "cannot determine system time");
    else
      _now = new_now;
  }

  class timeout : public system_time
  {
  public:
    typedef boost::function<void ()>      handler;
    typedef std::multimap<time_t,handler> handler_map;
    typedef handler_map::iterator         iterator;
    typedef handler_map::value_type       context;
    typedef std::pair<time_t,iterator>    event;

    event at(time_t const & to, handler const & f)
    {
      BOOST_ASSERT(f);
      BOOST_ASSERT(to >= now());
      return event(to, _the_map.insert(context(to, f)));
    }

    event in(second_t seconds, handler const & f)
    {
      return at(time_t(now() + seconds), f);
    }

    void cancel(event const & to)
    {
      if (to.first >= now()) _the_map.erase(to.second);
    }

    bool empty()
    {
      return _the_map.empty();
    }

    size_t deliver(second_t * next_in_seconds = 0)
    {
      update();
      size_t n( 0u );
      while (!empty())
      {
        iterator i( _the_map.begin() );
        if (i->first < now())
        {
          ++n;
          i->second();
          _the_map.erase(i);
        }
        else
        {
          if (next_in_seconds)
            *next_in_seconds = static_cast<second_t>(i->first - now()) + 1u;
          break;
        }
      }
      return n;
    }

  private:
    handler_map _the_map;
  };

} // namespace ioxx

#include <boost/test/minimal.hpp>

int test_main(int, char**)
{
  ioxx::timeout timer;
}
