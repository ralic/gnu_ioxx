#include <ioxx/error.hpp>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <sys/time.h>
#include <map>
#include <boost/strong_typedef.hpp>

namespace ioxx
{
  using std::time_t;
  typedef unsigned int seconds_t;

  class system_time : private boost::noncopyable
  {
  public:
    system_time()                       { update(); }

    time_t const &  as_time_t() const  { return _now.tv_sec; }
    timeval const & as_timeval() const { return _now; }

    void update()
    {
      throw_errno_if_minus1_<int>( "system call gettimeofday(2) failed"
                                 , boost::bind(gettimeofday, &_now, static_cast<struct timezone *>(0))
                                 );
    }

  private:
    timeval _now;
  };

  class scheduler : boost::noncopyable
  {
  public:
    typedef boost::function<void ()>            task;
    typedef std::multimap<time_t,task>          task_queue;
    typedef task_queue::iterator                queue_iterator;
    typedef task_queue::value_type              queue_entry;
    typedef std::pair<time_t,queue_iterator>    task_id;

    task_id at(time_t to, task const & f)
    {
      BOOST_ASSERT(f);
      return task_id(to, _queue.insert(queue_entry(to, f)));
    }

    bool cancel(task_id & tid)
    {
      queue_iterator i( _queue.begin() );
      if (tid.first > i->first)
      {
        _queue.erase(tid.second);
        tid.first = static_cast<time_t>(0);
        return true;
      }
      while (i->first == tid.first)
      {
        if (i == tid.second)
        {
          _queue.erase(tid.second);
          tid.first = static_cast<time_t>(0);
          return true;
        }
        else
          ++i;
      }
      return false;
    }

    bool cancel(task_id & tid, time_t now)
    {
      if (tid.first > now)
      {
        BOOST_ASSERT(_queue.begin()->first <= tid.first);
        _queue.erase(tid.second);
        tid.first = static_cast<time_t>(0);
        return true;
      }
      else
        return cancel(tid);
    }

    bool empty() const
    {
      return _queue.empty();
    }

    seconds_t run(time_t now)
    {
      queue_iterator i;
      for (task f; !empty(); f.clear())
      {
        i = _queue.begin();
        if (i->first <= now)
        {
          f.swap(i->second);
          try { _queue.erase(i); }
          catch(...)
          {
            f.swap(i->second);
            throw;
          }
          f();
        }
        else
          return static_cast<seconds_t>(i->first - now);
      }
      return 0;
    }

  private:
    task_queue _queue;
  };

} // namespace ioxx

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE( basic_system_time_test )
{
  ioxx::system_time now;
  BOOST_CHECK_EQUAL(now.as_time_t(), std::time(0));
  BOOST_CHECK_EQUAL(now.as_time_t(), now.as_timeval().tv_sec);
}

static size_t dummy_was_called = 0u;
void dummy_function() { ++dummy_was_called; }

BOOST_AUTO_TEST_CASE( basic_scheduler_test )
{
  ioxx::system_time now;
  ioxx::scheduler schedule;
  BOOST_CHECK(schedule.empty());
  BOOST_CHECK_EQUAL(schedule.run(now.as_time_t()), 0u);
  BOOST_CHECK_EQUAL(dummy_was_called, 0u);
  schedule.at(now.as_time_t(), dummy_function);
  schedule.at(now.as_time_t() + 1u, dummy_function);
  ioxx::scheduler::task_id tid( schedule.at(now.as_time_t() + 5u, dummy_function) );
  schedule.at(now.as_time_t() + 2u, boost::bind(&ioxx::scheduler::cancel, &schedule, tid));
  ioxx::seconds_t delay( schedule.run(now.as_time_t()) );
  BOOST_CHECK_EQUAL(delay, 1u);
  BOOST_CHECK_EQUAL(dummy_was_called, 1u);
  sleep(delay); now.update();
  delay = schedule.run(now.as_time_t());
  BOOST_CHECK_EQUAL(dummy_was_called, 2u);
  BOOST_CHECK_EQUAL(delay, 1u);
  sleep(delay); now.update();
  delay = schedule.run(now.as_time_t());
  BOOST_CHECK_EQUAL(dummy_was_called, 2u);
  BOOST_CHECK_EQUAL(delay, 0u);
  BOOST_CHECK(schedule.empty());
}