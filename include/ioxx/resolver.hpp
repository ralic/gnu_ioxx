#ifndef IOXX_RESOLVER_HPP_INCLUDED_2008_04_20
#define IOXX_RESOLVER_HPP_INCLUDED_2008_04_20

#include "config.hpp"

#if defined(IOXX_HAVE_ADNS)
#  include "resolver/adns.hpp"
namespace ioxx { typedef resolver::adns<> default_resolver; }
#else
#  error "No asynchronous DNS resolver available."
#endif

#endif // IOXX_RESOLVER_HPP_INCLUDED_2008_04_20
