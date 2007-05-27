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

#include "ioxx/type/system-time.hpp"
#include "ioxx/type/system-error.hpp"

std::time_t ioxx::system_time::_now = ioxx::time_t(0);

void ioxx::system_time::update()
{
  using namespace std;
  time_t new_now;
  if (time(&new_now) == std::time_t(-1))
    throw system_error("cannot determine system time");
  else
    _now = new_now;
}
