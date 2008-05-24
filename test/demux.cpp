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
#include <ioxx/socket.hpp>
#include <functional>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>

template <class T>
struct demux_concept
{
  void constraints()
  {
    typedef T                                   demux;
    typedef typename demux::socket              socket;
    typedef typename demux::socket::native_t    native_socket_t;
    typedef typename socket::event_set          event_set;

    using namespace boost;
    function_requires< DefaultConstructibleConcept<demux> >();
    function_requires< DefaultConstructibleConcept<event_set> >();

    event_set ev1, ev2;
    ev1 |= ev2; ev1 = ev1 | ev2;
    ev1 &= ev2; ev1 = ev1 & ev2;
    /* ev1 = ~ev2; */
    ev1 = socket::no_events;
    ev1 = socket::readable;
    ev1 = socket::writable;
    ev1 = socket::pridata;

    demux dmx;
    native_socket_t sock;
    bool b( dmx.empty() );
    b = dmx.pop_event(sock, ev1);
    dmx.wait(static_cast<ioxx::seconds_t>(0));

    socket s2(dmx, sock), s3(dmx, sock, ev1);
    s2.request(ev1);
    ioxx::system_socket & s4( s3 );
    ignore_unused_variable_warning(s4);
  }
};

struct demux_archetype
{
  struct socket : public ioxx::system_socket
  {
    typedef short event_set;
    static event_set const no_events = 0;
    static event_set const readable  = 1 << 0;
    static event_set const writable  = 1 << 1;
    static event_set const pridata   = 1 << 2;

    socket(demux_archetype &, ioxx::native_socket_t s) : ioxx::system_socket(s) { }
    socket(demux_archetype &, ioxx::native_socket_t s, event_set) : ioxx::system_socket(s) { }

    void request(event_set) { }
  };

  typedef socket::event_set     event_set;

  demux_archetype() { throw std::logic_error("demux_archetype is not a valid implementation"); }

  bool empty() { return true; }
  bool pop_event(ioxx::native_socket_t & s, socket::event_set & ev)  { return false; }
  void wait(ioxx::seconds_t to) { }
};

demux_archetype::socket::event_set const demux_archetype::socket::readable;
demux_archetype::socket::event_set const demux_archetype::socket::writable;
demux_archetype::socket::event_set const demux_archetype::socket::pridata;

template <class Demux>
void use_standard_event_set_operators()
{
  typedef typename Demux::socket        socket;
  typedef typename socket::event_set    event_set;

  using namespace boost;
  function_requires< AssignableConcept<event_set> >();
  function_requires< SGIAssignableConcept<event_set> >();
  function_requires< DefaultConstructibleConcept<event_set> >();
  function_requires< EqualityComparableConcept<event_set> >();

  BOOST_REQUIRE(!socket::no_events);

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
  ioxx::time_of_day now;
  ioxx::time_t const pre_sleep( now.as_time_t() );
  Demux demux;
  BOOST_REQUIRE(demux.empty());
  demux.wait(1u);
  now.update();
  ioxx::time_t const post_sleep( now.as_time_t() );
  BOOST_REQUIRE_PREDICATE(std::greater_equal<ioxx::time_t>(), (post_sleep)(pre_sleep));
}

template <class Demux>
void test_demux()
{
  boost::function_requires< demux_concept<Demux> >();
  use_standard_event_set_operators<Demux>();
  use_demuxer_for_sleeping<Demux>();
}

BOOST_AUTO_TEST_CASE( test_demux_archetype )
{
  BOOST_REQUIRE_THROW(test_demux<demux_archetype>(), std::logic_error);
}

#if defined(IOXX_HAVE_EPOLL) && IOXX_HAVE_EPOLL
#  include <ioxx/detail/epoll.hpp>

BOOST_AUTO_TEST_CASE( test_epoll_demux )
{
  test_demux<ioxx::detail::epoll>();
}
#endif

#if defined(IOXX_HAVE_POLL) && IOXX_HAVE_POLL
#  include <ioxx/detail/poll.hpp>

BOOST_AUTO_TEST_CASE( test_poll_demux )
{
  test_demux< ioxx::detail::poll<> >();
}
#endif

#if defined(IOXX_HAVE_SELECT) && IOXX_HAVE_SELECT
#  include <ioxx/detail/select.hpp>

BOOST_AUTO_TEST_CASE( test_select_demux )
{
  test_demux<ioxx::detail::select>();
}
#endif
