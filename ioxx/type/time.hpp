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

#ifndef IOXX_TYPE_TIME_INCLUDED
#define IOXX_TYPE_TIME_INCLUDED

#include <boost/compatibility/cpp_c_headers/ctime>

namespace ioxx
{
  using std::time_t;

  typedef unsigned int second_t;
}

#endif // IOXX_TYPE_TIME_INCLUDED
