/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_TIME_OF_DAY_HPP_INCLUDED
#define IOXX_TIME_OF_DAY_HPP_INCLUDED

#include <ctime>
#include "error.hpp"

/**
 *  \brief A caching interface to the time of day.
 */
class TimeOfDay
{
  time_t now;

public:
  TimeOfDay()			{ this->update(); }
  operator time_t () const	{ return now; }
  void update()
  {
    if (std::time(&now) == static_cast<time_t>(-1))
      throw ioxx::system::error("time(2) failed");
  }
};

#endif // IOXX_TIME_OF_DAY_HPP_INCLUDED
