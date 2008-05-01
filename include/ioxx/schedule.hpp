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

    task_id at(time_t to, task const & f)
    {
      return task_id(to, _queue.insert(queue_entry(to, f)));
    }

    void unsafe_cancel(task_id & tid)
    {
      BOOST_ASSERT(tid.first != 0);
      _queue.erase(tid.second);
      tid.first = static_cast<time_t>(0);
    }

    bool cancel(task_id & tid)
    {
      if (tid.first == 0) return false;
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

    bool cancel(task_id & tid, time_t now)
    {
      if (tid.first > now)
      {
        BOOST_ASSERT(_queue.begin()->first <= tid.first);
        unsafe_cancel(tid);
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
      while (!empty())
      {
        queue_iterator i( _queue.begin() );
        if (i->first <= now)
        {
          task f(i->second);
          _queue.erase(i);
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

#endif // IOXX_SCHEDULE_HPP_INCLUDED_2008_04_20
