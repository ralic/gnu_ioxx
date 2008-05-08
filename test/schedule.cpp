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

#include <ioxx/time.hpp>
#include <ioxx/schedule.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <functional>

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
  typedef ioxx::schedule<> scheduler;

  time now;
  scheduler schedule(now.as_time_t());
  size_t dummy_call_counter( 0u );
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(), 0u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 0u);
  schedule.at(now.as_time_t(), dummy(dummy_call_counter));
  schedule.in(1u, dummy(dummy_call_counter));
  scheduler::task_id tid( schedule.in(5u, dummy(dummy_call_counter)) );
  seconds_t delay( schedule.run() );
  BOOST_REQUIRE_EQUAL(delay, 1u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 1u);
  sleep(delay); now.update();
  delay = schedule.run();
  BOOST_REQUIRE_EQUAL(delay, 4u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 2u);
  schedule.unsafe_cancel(tid);
  delay = schedule.run();
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 2u);
  BOOST_REQUIRE_EQUAL(delay, 0u);
  BOOST_REQUIRE(schedule.empty());
}

BOOST_AUTO_TEST_CASE( test_schedule_timeout )
{
  ioxx::time now;
  typedef ioxx::schedule<> scheduler;
  scheduler schedule(now.as_time_t());
  size_t dummy_call_counter( 0u );
  {
    scheduler::timeout timeout( schedule );
    BOOST_REQUIRE_EQUAL(dummy_call_counter, 0u);
    timeout.at(now.as_time_t(), dummy(dummy_call_counter));
    schedule.run();
    BOOST_REQUIRE_EQUAL(dummy_call_counter, 1u);
    timeout.in(1u, dummy(dummy_call_counter));
  }
  schedule.run();
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 1u);
}

static size_t dummy_was_called = 0u;
void dummy_function() { ++dummy_was_called; }

BOOST_AUTO_TEST_CASE( basic_schedule_test )
{
  using ioxx::time;
  using ioxx::seconds_t;
  typedef ioxx::schedule<> scheduler;
  time now;
  scheduler schedule(now.as_time_t());
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(), 0u);
  BOOST_REQUIRE_EQUAL(dummy_was_called, 0u);
  schedule.at(now.as_time_t(), dummy_function);
  schedule.in(1u, dummy_function);
  scheduler::task_id tid( schedule.in(5u, dummy_function) );
  schedule.in(2u, boost::bind(&scheduler::cancel, &schedule, tid));
  seconds_t delay( schedule.run() );
  BOOST_REQUIRE_EQUAL(delay, 1u);
  BOOST_REQUIRE_EQUAL(dummy_was_called, 1u);
  sleep(delay); now.update();
  delay = schedule.run();
  BOOST_REQUIRE_EQUAL(dummy_was_called, 2u);
  BOOST_REQUIRE_PREDICATE(std::less_equal<seconds_t>(), (delay)(1u));
  sleep(delay); now.update();
  delay = schedule.run();
  BOOST_REQUIRE_EQUAL(dummy_was_called, 2u);
  BOOST_REQUIRE_EQUAL(delay, 0u);
  BOOST_REQUIRE(schedule.empty());
}
