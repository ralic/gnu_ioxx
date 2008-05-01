#include <ioxx/time.hpp>
#include <ioxx/schedule.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <functional>

static size_t dummy_was_called = 0u;
void dummy_function() { ++dummy_was_called; }

BOOST_AUTO_TEST_CASE( basic_schedule_test )
{
  using ioxx::time;
  using ioxx::seconds_t;
  typedef ioxx::schedule<> scheduler;
  time now;
  scheduler schedule;
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(now.as_time_t()), 0u);
  BOOST_REQUIRE_EQUAL(dummy_was_called, 0u);
  schedule.at(now.as_time_t(), dummy_function);
  schedule.at(now.as_time_t() + 1u, dummy_function);
  scheduler::task_id tid( schedule.at(now.as_time_t() + 5u, dummy_function) );
  schedule.at(now.as_time_t() + 2u, boost::bind(&scheduler::cancel, &schedule, tid));
  seconds_t delay( schedule.run(now.as_time_t()) );
  BOOST_REQUIRE_EQUAL(delay, 1u);
  BOOST_REQUIRE_EQUAL(dummy_was_called, 1u);
  sleep(delay); now.update();
  delay = schedule.run(now.as_time_t());
  BOOST_REQUIRE_EQUAL(dummy_was_called, 2u);
  BOOST_REQUIRE_PREDICATE(std::less_equal<int>(), (delay)(1u));
  sleep(delay); now.update();
  delay = schedule.run(now.as_time_t());
  BOOST_REQUIRE_EQUAL(dummy_was_called, 2u);
  BOOST_REQUIRE_EQUAL(delay, 0u);
  BOOST_REQUIRE(schedule.empty());
}

class dummy
{
public:
  dummy(size_t & cnt) : _cnt(&cnt)      { }
  void operator() () const              { ++(*_cnt); }

private:
  size_t * _cnt;
};

BOOST_AUTO_TEST_CASE( dummy_schedule_test )
{
  using ioxx::time;
  using ioxx::seconds_t;
  typedef ioxx::schedule<dummy> scheduler;

  time now;
  scheduler schedule;
  size_t dummy_call_counter( 0u );
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(now.as_time_t()), 0u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 0u);
  schedule.at(now.as_time_t(), dummy(dummy_call_counter));
  schedule.at(now.as_time_t() + 1u, dummy(dummy_call_counter));
  scheduler::task_id tid( schedule.at(now.as_time_t() + 5u, dummy(dummy_call_counter)) );
  seconds_t delay( schedule.run(now.as_time_t()) );
  BOOST_REQUIRE_EQUAL(delay, 1u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 1u);
  sleep(delay); now.update();
  delay = schedule.run(now.as_time_t());
  BOOST_REQUIRE_EQUAL(delay, 4u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 2u);
  schedule.unsafe_cancel(tid);
  delay = schedule.run(now.as_time_t());
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 2u);
  BOOST_REQUIRE_EQUAL(delay, 0u);
  BOOST_REQUIRE(schedule.empty());
}
