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

#ifndef IOXX_TIMEOUT_HPP_INCLUDED
#define IOXX_TIMEOUT_HPP_INCLUDED

#include "type/system-time.hpp"
#include <map>
#include <boost/function.hpp>

namespace ioxx
{
  class timeout : public system_time
  {
  public:
    typedef boost::function<void ()>      handler;
    typedef std::multimap<time_t,handler> handler_map;
    typedef handler_map::iterator         iterator;
    typedef handler_map::value_type       context;
    typedef std::pair<time_t,iterator>    event;

    event at(time_t const & to, handler const & f)
    {
      BOOST_ASSERT(f);
      BOOST_ASSERT(to >= now());
      return event(to, _the_map.insert(context(to, f)));
    }

    event in(unsigned int seconds, handler const & f)
    {
      return at(now() + seconds, f);
    }

    void cancel(event const & to)
    {
      if (to.first >= now()) _the_map.erase(to.second);
    }

    bool empty()
    {
      return _the_map.empty();
    }

    size_t deliver(unsigned int * next_in_seconds = 0)
    {
      update();
      size_t n( 0u );
      while (!empty())
      {
        iterator i( _the_map.begin() );
        if (i->first < now())
        {
          ++n;
          i->second();
          _the_map.erase(i);
        }
        else
        {
          if (next_in_seconds)
            *next_in_seconds = static_cast<unsigned int>(i->first - now()) + 1u;
          break;
        }
      }
      return n;
    }

  private:
    handler_map _the_map;
  };

} // namespace ioxx

#endif // IOXX_TIMEOUT_HPP_INCLUDED
