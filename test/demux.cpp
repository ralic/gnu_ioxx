#include "ioxx/timer.hpp"
#include <functional>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>

template <class Demux>
void use_standard_event_set_operators()
{
  typedef typename Demux::socket        socket;
  typedef typename socket::event_set    event_set;

  event_set ev( socket::readable );
  BOOST_REQUIRE_EQUAL(ev, socket::readable);
  BOOST_REQUIRE(ev & socket::readable);
  BOOST_REQUIRE(!(ev & socket::writable));
  BOOST_REQUIRE(!(ev & socket::pridata));

  ev |= socket::writable;
  BOOST_REQUIRE_EQUAL((int)(ev), (int)(socket::readable) | (int)(socket::writable));
  BOOST_REQUIRE(ev & socket::readable);
  BOOST_REQUIRE(ev & socket::writable);
  BOOST_REQUIRE(!(ev & socket::pridata));

  ev = socket::writable | socket::pridata;
  BOOST_REQUIRE_EQUAL((int)(ev), (int)(socket::pridata) | (int)(socket::writable));
  BOOST_REQUIRE(ev & socket::pridata);
  BOOST_REQUIRE(ev & socket::writable);
  BOOST_REQUIRE(!(ev & socket::readable));
}

template <class Demux>
void use_demuxer_for_sleeping()
{
  ioxx::timer now;
  ioxx::time_t const pre_sleep( now.as_time_t() );
  Demux demux;
  BOOST_REQUIRE(demux.empty());
  demux.wait(1u);
  now.update();
  ioxx::time_t const post_sleep( now.as_time_t() );
  BOOST_REQUIRE_PREDICATE(std::greater_equal<int>(), (post_sleep - pre_sleep)(1));
}

template <class Demux>
void test_demux()
{
  use_standard_event_set_operators<Demux>();
  use_demuxer_for_sleeping<Demux>();
}

#if defined(IOXX_HAVE_EPOLL)
#  include "ioxx/demux/epoll.hpp"

BOOST_AUTO_TEST_CASE( test_epoll_demux )
{
  test_demux<ioxx::demux::epoll>();
}
#endif

#if defined(IOXX_HAVE_POLL)
#  include "ioxx/demux/poll.hpp"

BOOST_AUTO_TEST_CASE( test_poll_demux )
{
  test_demux< ioxx::demux::poll<> >();
}
#endif

#if defined(IOXX_HAVE_SELECT)
#  include "ioxx/demux/select.hpp"

BOOST_AUTO_TEST_CASE( test_select_demux )
{
  test_demux<ioxx::demux::select>();
}
#endif

