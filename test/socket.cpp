/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#include "ioxx/probe.hpp"
#include <iostream>
#include <boost/function.hpp>
#include <boost/assert.hpp>
#include <deque>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

struct mutex
{
  struct scoped_lock
  {
    scoped_lock(mutex &) { }
  };
};

class IO
{
public:
  typedef boost::function<void ()> task;

  void push_back(task const & t)
  {
    mutex_type::scoped_lock l( _mutex );
    _queue.push_back(t);
  }

  bool run_once()
  {
    task t;
    {
      mutex::scoped_lock l( _mutex );
      if (_queue.empty()) return false;
      t.swap( _queue.front() );
      _queue.pop_front();
    }
    BOOST_ASSERT( t );
    t();
    return true;
  }

  void run()
  {
    while(run_once()) /**/;
  }

private:
  typedef mutex            mutex_type;
  typedef std::deque<task> queue_type;

  mutex         _mutex;
  queue_type    _queue;
};

class probe
{
public:
};

class basic_socket
{
public:
  basic_socket(int s) : _fd(s) { }

private:
  probe *       _probe;
  int           _fd;
};

BOOST_AUTO_TEST_CASE( test_io_probe )
{
  basic_socket sin(STDIN_FILENO);
}
