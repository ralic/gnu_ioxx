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

#include "ioxx/iovector.hpp"
#include <boost/static_assert.hpp>
#include <boost/concept_check.hpp>
using namespace ioxx;

// don't trust the compiler to get inheritance right
BOOST_STATIC_ASSERT(sizeof(iovector<char>) == sizeof(system_iovector));

int main(int, char**)
{
  boost::function_requires<
    boost::Mutable_RandomAccessContainerConcept<
      iovector<char>
    >
  >();

  return 0;
}
