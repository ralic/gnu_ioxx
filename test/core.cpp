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
#include <ioxx/dispatch.hpp>
#include <ioxx/dns.hpp>

namespace ioxx
{
  class core : public dispatch<>
  {
  public:
    typedef schedule<>                  schedule;
    typedef schedule::task              task;
    typedef schedule::task_id           task_id;
    typedef schedule::timeout           timeout;
    typedef dispatch<>                  dispatch;
    typedef boost::function0<void>      handler;

    class socket : public dispatch::socket
    {
    public:
      socket(core & io, native_socket_t sock)
      : dispatch::socket( io, sock, boost::bind(&socket::run, this, _1), dispatch::socket::no_events)
      , _to_in(io._schedule), _to_out(io._schedule), _to_pri(io._schedule)
      {
      }

      void reset()
      {
        _in.clear();  _to_in.cancel();
        _out.clear(); _to_out.cancel();
        _pri.clear(); _to_pri.cancel();
        register_events();
      }

      void on_input(handler const & f)
      {
        set(_in, _to_in, f, 0u, handler());
      }

      void on_input(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_in, _to_in, f, tout, on_timeout);
      }

      void on_output(handler const & f)
      {
        set(_out, _to_out, f, 0u, handler());
      }

      void on_output(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_out, _to_out, f, tout, on_timeout);
      }

      void on_pridata(handler const & f)
      {
        set(_pri, _to_pri, f, 0u, handler());
      }

      void on_pridata(handler const & f, seconds_t tout, handler const & on_timeout)
      {
        set(_pri, _to_pri, f, tout, on_timeout);
      }

    protected:
      core & context() { return static_cast<core &>(dispatch::socket::context()); }

    private:
      handler _in, _out, _pri;
      timeout _to_in, _to_out, _to_pri;

      void set(handler & f_, timeout & to_f_, handler f, seconds_t tout, handler const & on_timeout)
      {
        BOOST_ASSERT(tout == 0u || on_timeout);
        timeout to( context()._schedule );
        if (tout && on_timeout) to.reset(context()._now.as_time_t() + tout, on_timeout);
        to.swap(to_f_);
        f.swap(f_);
        if (f_ && !f)
        {
          try { register_events(); }
          catch(...)
          {
            to.swap(to_f_);
            f.swap(f_);
            throw;
          }
        }
      }

      void register_events()
      {
        dispatch::socket::event_set req( dispatch::socket::no_events );
        if (_in)  req |= dispatch::socket::readable;
        if (_out) req |= dispatch::socket::writable;
        if (_pri) req |= dispatch::socket::pridata;
        this->request(req);
      }

      void run(dispatch::socket::event_set ev)
      {
        if (ev & dispatch::socket::readable && _in)
        {
          handler f;
          _to_in.cancel();
          f.swap(_in);
          f();
        }
        if (ev & dispatch::socket::writable && _out)
        {
          handler f;
          _to_out.cancel();
          f.swap(_out);
          f();
        }
        if (ev & dispatch::socket::pridata && _pri)
        {
          handler f;
          _to_pri.cancel();
          f.swap(_pri);
          f();
        }
        if (ev != dispatch::socket::no_events)
          register_events();
      }
    };

    task_id at(time_t ts, task const & f)       { return _schedule.at(ts, f); }
    task_id in(seconds_t to, task const & f)    { return _schedule.at(_now.as_time_t() + to, f); }

    void run()
    {
      for (;;)
      {
        dispatch::run();
        seconds_t to( _schedule.run(_now.as_time_t()) );
        if (_schedule.empty())
        {
          if (dispatch::empty()) break;
          else                   to = dispatch::max_timeout();
        }
        dispatch::wait(to);
        _now.update();
      }
    }

  private:
    time        _now;
    schedule    _schedule;
  };

} // namespace ioxx

///// test cases //////////////////////////////////////////////////////////////

#include <ioxx/acceptor.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

class echo : public boost::enable_shared_from_this<echo>
{
  ioxx::core::socket            _sock;
  boost::array<char,1024>       _buf;
  size_t                        _len;
  size_t                        _gap;

  echo(ioxx::core & io, ioxx::native_socket_t s) : _sock(io, s), _len(0u), _gap(0u)
  {
    BOOST_REQUIRE(_buf.size());
  }

  void input()
  {
    BOOST_REQUIRE_EQUAL(_len, 0u);
    char * const data_end( _sock.read(_buf.begin(), _buf.end()) );
    if (!data_end) return;
    BOOST_ASSERT(_buf.begin() < data_end);
    _len = static_cast<size_t>(data_end - _buf.begin());
    _sock.on_output(boost::bind(&echo::output, shared_from_this()), 5u, boost::bind(&echo::reset, shared_from_this()));
  }

  void output()
  {
    BOOST_REQUIRE(_len);
    BOOST_REQUIRE(_gap + _len <= _buf.size());
    char const * const new_begin( _sock.write(&_buf[_gap], &_buf[_gap + _len]) );
    BOOST_REQUIRE(new_begin);
    BOOST_REQUIRE(_buf.begin() < new_begin);
    size_t const n(new_begin - _buf.begin());
    _gap  += n;
    _len -= n;
    if (_len == 0u)
    {
      _gap = 0u;
      _sock.on_input(boost::bind(&echo::input, shared_from_this()), 5u, boost::bind(&echo::reset, shared_from_this()));
    }
  }

  void reset() { _sock.reset(); }

public:
  ~echo() { IOXX_TRACE_MSG("destroy echo handler"); }

  static void accept(ioxx::core & io, ioxx::native_socket_t s, ioxx::core::socket::address const & addr)
  {
    IOXX_TRACE_SOCKET(s, "start echo handler for peer " << addr.show());
    boost::shared_ptr<echo> ptr;
    ptr.reset( new echo(io, s) );
    ptr->_sock.on_input(boost::bind(&echo::input, ptr));
  }
};

BOOST_AUTO_TEST_CASE( test_echo_handler )
{
  typedef ioxx::core::dispatch          dispatch;
  typedef ioxx::core::socket            socket;
  typedef socket::address               address;
  typedef socket::endpoint              endpoint;
  typedef ioxx::acceptor<dispatch>      acceptor;
  typedef boost::scoped_ptr<acceptor>   acceptor_ptr;

  using boost::bind;
  using boost::ref;

  ioxx::core    io;
  dispatch      disp;
  acceptor_ptr  ls;
  ls.reset( new acceptor( io, endpoint("127.0.0.1", "8080")
                        , bind(&echo::accept, ref(io), _1, _2)
                        ));
  IOXX_TRACE_SOCKET(*ls, "accepting connections on port 8080");
  io.in(5u, bind(&acceptor_ptr::reset, ref(ls), static_cast<acceptor*>(0)));
  io.run();
  IOXX_TRACE_MSG("shutting down");
}
