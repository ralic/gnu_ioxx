#include "ioxx/timer.hpp"
#include "ioxx/demux/epoll.hpp"
#include "ioxx/demux/select.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <functional>

// BOOST_AUTO_TEST_CASE( test_socket_event_operators )
// {
//   using namespace ioxx;
//
//   socket_event ev( ev_readable );
//   BOOST_REQUIRE_EQUAL(ev, ev_readable);
//   BOOST_REQUIRE(ev & ev_readable);
//   BOOST_REQUIRE(!(ev & ev_writable));
//   BOOST_REQUIRE(!(ev & ev_pridata));
//
//   ev |= ev_writable;
//   BOOST_REQUIRE_EQUAL((int)(ev), (int)(ev_readable) | (int)(ev_writable));
//   BOOST_REQUIRE(ev & ev_readable);
//   BOOST_REQUIRE(ev & ev_writable);
//   BOOST_REQUIRE(!(ev & ev_pridata));
//
//   ev = ev_writable | ev_pridata;
//   BOOST_REQUIRE_EQUAL((int)(ev), (int)(ev_pridata) | (int)(ev_writable));
//   BOOST_REQUIRE(ev & ev_pridata);
//   BOOST_REQUIRE(ev & ev_writable);
//   BOOST_REQUIRE(!(ev & ev_readable));
// }

template <class demux_type>
void use_demuxer_for_sleeping()
{
  ioxx::timer now;
  ioxx::time_t const pre_sleep( now.as_time_t() );
  demux_type demux;
  BOOST_REQUIRE(demux.empty());
  demux.wait(1u);
  now.update();
  ioxx::time_t const post_sleep( now.as_time_t() );
  BOOST_REQUIRE_PREDICATE(std::greater_equal<int>(), (post_sleep - pre_sleep)(1));
}

// BOOST_AUTO_TEST_CASE( test_that_epoll_can_be_used_as_sleep )
// {
//   use_demuxer_for_sleeping<ioxx::demux::epoll>();
// }

BOOST_AUTO_TEST_CASE( test_that_epoll_can_be_used_as_sleep )
{
  use_demuxer_for_sleeping<ioxx::demux::select>();
}
