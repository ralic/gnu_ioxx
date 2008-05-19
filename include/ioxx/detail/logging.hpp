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

#ifndef IOXX_DETAIL_LOGGING_HPP_INCLUDED_2008_05_19
#define IOXX_DETAIL_LOGGING_HPP_INCLUDED_2008_05_19

#include <ioxx/detail/config.hpp>
#include <ostream>

#if defined(IOXX_HAVE_LOGGING) && IOXX_HAVE_LOGGING
#  include <logxx.hpp>
#else
#  define LOGXX_DEFINE_TARGET(id)
#  define LOGXX_GET_TARGET(target,channel)
#  define LOGXX_CONFIGURE_TARGET(target,channel,pri)
#  define LOGXX_MSG(target,pri,msg)
#  define LOGXX_MSG_FATAL(target,msg)
#  define LOGXX_MSG_CRITICAL(target,msg)
#  define LOGXX_MSG_ERROR(target,msg)
#  define LOGXX_MSG_WARNING(target,msg)
#  define LOGXX_MSG_NOTICE(target,msg)
#  define LOGXX_MSG_INFO(target,msg)
#  define LOGXX_MSG_DEBUG(target,msg)
#  define LOGXX_MSG_TRACE(target,msg)
#  define LOGXX_SCOPE_NAME
#  define LOGXX_SCOPE(channel)
#  define LOGXX_FATAL(msg)
#  define LOGXX_CRITICAL(msg)
#  define LOGXX_ERROR(msg)
#  define LOGXX_WARNING(msg)
#  define LOGXX_NOTICE(msg)
#  define LOGXX_INFO(msg)
#  define LOGXX_DEBUG(msg)
#  define LOGXX_TRACE(msg)
#endif

#endif // IOXX_DETAIL_LOGGING_HPP_INCLUDED_2008_05_19
