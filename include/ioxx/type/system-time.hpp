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

#ifndef IOXX_TYPE_SYSTEM_TIME_INCLUDED
#define IOXX_TYPE_SYSTEM_TIME_INCLUDED

#include "time.hpp"
#include <boost/noncopyable.hpp>

namespace ioxx
{
  class system_time : private boost::noncopyable
  {
  public:
    system_time()                       { update(); }

    static time_t const &  now()        { return _now; }
    static void            update();

  private:
    static time_t _now;
  };
}

#endif // IOXX_TYPE_SYSTEM_TIME_INCLUDED
