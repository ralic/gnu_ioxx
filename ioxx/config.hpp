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

#ifndef IOXX_CONFIG_HPP_INCLUDED
#define IOXX_CONFIG_HPP_INCLUDED

#include <boost/config.hpp>

#ifndef NDEBUG
#  include <iostream>
#  include <boost/assert.hpp>
#  define IOXX_MSG_TRACE(msg)           std::cerr << "[ioxx." <<__func__ << "] " << msg << std::endl
#  define IOXX_ASSERT(exp)              BOOST_ASSERT(exp)
#  define IOXX_ASSERT_EQ(lhs,rhs)       IOXX_ASSERT((lhs) == (rhs))
#  define IOXX_ASSERT_LT(lhs,rhs)       IOXX_ASSERT((lhs) <  (rhs))
#else
#  define IOXX_MSG_TRACE(msg)           ((void)(0))
#  define IOXX_ASSERT(exp)              ((void)(0))
#  define IOXX_ASSERT_EQ(lhs,rhs)       ((void)(0))
#  define IOXX_ASSERT_LT(lhs,rhs)       ((void)(0))
#endif

#ifdef _MSC_VER
#  if defined(IOXX_BUILDING_DLL)
#    define IOXX_DLL_EXPORT             __declspec(dllexport)
#  else
#    define IOXX_DLL_EXPORT             __declspec(dllimport)
#  endif
#  define IOXX_DLL_LOCAL
#else
#  if defined(__GNUC__) && !defined(__ICC)
#    define IOXX_DLL_EXPORT             __attribute__ ((visibility("default")))
#    define IOXX_DLL_LOCAL              __attribute__ ((visibility("hidden")))
#  else
#    define IOXX_DLL_EXPORT
#    define IOXX_DLL_LOCAL
#  endif
#endif

#ifdef __GNUC__
#  define IOXX_NORETURN_DECL            __attribute__ ((noreturn))
#else
#  define IOXX_NORETURN_DECL
#endif

#ifdef NDEBUG
#  define IOXX_NOTHROW_DECL
#else
#  define IOXX_NOTHROW_DECL             throw()
#endif

#endif // IOXX_CONFIG_HPP_INCLUDED
