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
  using ioxx::time_of_day;
  using ioxx::seconds_t;
  typedef ioxx::schedule<> scheduler;

  time_of_day now;
  scheduler schedule(now.current_time_t());
  size_t dummy_call_counter( 0u );
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(), 0u);
  BOOST_REQUIRE_EQUAL(dummy_call_counter, 0u);
  schedule.at(now.current_time_t(), dummy(dummy_call_counter));
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
  ioxx::time_of_day now;
  typedef ioxx::schedule<> scheduler;
  scheduler schedule(now.current_time_t());
  size_t dummy_call_counter( 0u );
  {
    scheduler::timeout timeout( schedule );
    BOOST_REQUIRE_EQUAL(dummy_call_counter, 0u);
    timeout.at(now.current_time_t(), dummy(dummy_call_counter));
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
  using ioxx::time_of_day;
  using ioxx::seconds_t;
  typedef ioxx::schedule<> scheduler;
  time_of_day now;
  scheduler schedule(now.current_time_t());
  BOOST_REQUIRE(schedule.empty());
  BOOST_REQUIRE_EQUAL(schedule.run(), 0u);
  BOOST_REQUIRE_EQUAL(dummy_was_called, 0u);
  schedule.at(now.current_time_t(), dummy_function);
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
