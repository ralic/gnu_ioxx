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

#ifndef IOXX_SCHEDULE_HPP_INCLUDED_2008_04_20
#define IOXX_SCHEDULE_HPP_INCLUDED_2008_04_20

#include <boost/function/function0.hpp>
#include <boost/noncopyable.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/assert.hpp>
#include <algorithm>
#include <map>

namespace ioxx
{
  using std::time_t;
  typedef unsigned int seconds_t;

  template < class Task      = boost::function0<void>
           , class Allocator = std::allocator< std::pair<time_t const, Task> >
           >
  class schedule : boost::noncopyable
  {
  public:
    typedef Task                                                        task;
    typedef std::multimap<time_t,task,std::less<time_t>,Allocator>      task_queue;
    typedef typename task_queue::iterator                               queue_iterator;
    typedef typename task_queue::value_type                             queue_entry;
    typedef std::pair<time_t,queue_iterator>                            task_id;

    class timeout : private boost::noncopyable
    {
    public:
      timeout(schedule & sched) : _sched(sched), _id(task_id(static_cast<time_t>(0), queue_iterator()))
      {
      }

      timeout(schedule & sched, time_t ts, task const & f) : _sched(sched), _id(_sched.at(ts, f))
      {
      }

      timeout(schedule & sched, seconds_t to, task const & f) : _sched(sched), _id(_sched.in(to, f))
      {
      }

      ~timeout()
      {
        _sched.cancel(_id);
      }

      void swap(timeout & other)
      {
        BOOST_ASSERT(&other._sched == &_sched);
        std::swap(other._id, _id);
      }

      bool cancel()
      {
        return _sched.cancel(_id);
      }

      bool at(time_t ts, task const & f)
      {
        bool const cancelled( _sched.cancel(_id) );
        _id = _sched.at(ts, f);
        return cancelled;
      }

      bool in(seconds_t to, task const & f)
      {
        return at(_sched.now() + to, f);
      }

      schedule &        get_schedule()          { return _sched; }
      schedule const &  get_schedule() const    { return _sched; }

    private:
      schedule & _sched;
      task_id    _id;
    };

    schedule(time_t const & now) : _now(now) { }

    time_t const & now() const { return _now; }

    task_id at(time_t ts, task const & f)
    {
      return task_id(ts, _queue.insert(queue_entry(ts, f)));
    }

    task_id in(seconds_t to, task const & f)
    {
      return at(_now + to, f);
    }

    void unsafe_cancel(task_id & tid)
    {
      BOOST_ASSERT(tid.first != 0);
      BOOST_ASSERT(!_queue.empty());
      _queue.erase(tid.second);
      tid.first = static_cast<time_t>(0);
    }

    bool cancel(task_id & tid)
    {
      if (tid.first == 0 || _queue.empty()) return false;
      queue_iterator i( _queue.begin() );
      if (tid.first > i->first)
      {
        unsafe_cancel(tid);
        return true;
      }
      while (i->first == tid.first)
      {
        if (i == tid.second)
        {
          unsafe_cancel(tid);
          return true;
        }
        else
          ++i;
      }
      return false;
    }

    bool empty() const
    {
      return _queue.empty();
    }

    seconds_t run()
    {
      while (!empty())
      {
        queue_iterator i( _queue.begin() );
        if (i->first <= _now)
        {
          task f(i->second);
          _queue.erase(i);
          f();
        }
        else
          return static_cast<seconds_t>(i->first - _now);
      }
      return 0;
    }

  private:
    task_queue          _queue;
    time_t const &      _now;
  };

} // namespace ioxx

#endif // IOXX_SCHEDULE_HPP_INCLUDED_2008_04_20
