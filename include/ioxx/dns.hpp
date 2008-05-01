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

#ifndef IOXX_DNS_HPP_INCLUDED_2008_04_20
#define IOXX_DNS_HPP_INCLUDED_2008_04_20

#include "detail/config.hpp"

#if defined(IOXX_BUILDING_DOCUMENTATION)
namespace ioxx { typedef implementation_defined dns; }
#elif defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
#  include "detail/adns.hpp"
namespace ioxx { typedef detail::adns<> dns; }
#else
#  error "No asynchronous DNS resolver available."
#endif

#endif // IOXX_DNS_HPP_INCLUDED_2008_04_20
