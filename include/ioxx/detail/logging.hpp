/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_DETAIL_LOGGING_HPP_INCLUDED_2010_02_23
#define IOXX_DETAIL_LOGGING_HPP_INCLUDED_2010_02_23

#include <ioxx/detail/config.hpp>
#include <ostream>

#if defined IOXX_HAVE_LOGGING && IOXX_HAVE_LOGGING
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

#endif // IOXX_DETAIL_LOGGING_HPP_INCLUDED_2010_02_23
