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

#ifndef IOXX_LOGGING_HPP_INCLUDED
#define IOXX_LOGGING_HPP_INCLUDED

#include <boost/log/log.hpp>

namespace ioxx
{
  namespace logging        /// The Boost.Log log targets used by ioxx.
  {
    /// \brief Use only during development.
    BOOST_DECLARE_LOG_DEBUG(catchall)

    /// \brief Trace memory allocations and deallocations.
    BOOST_DECLARE_LOG_DEBUG(memory)

    /// \brief To be used by ioxx::Probe implementations.
    BOOST_DECLARE_LOG_DEBUG(probe)

    /// \brief Init logging to stderr.
    ///
    /// Configure boost.log directly if you prefer. All our loggers's
    /// use the id pattern  "ioxx.*".
    void init_cerr();
  }
}

///@{
/// \brief Abstract logging API.
#define IOXX_DEBUG_MSG(chann)  BOOST_LOGL(::ioxx::logging::chann, dbg)
#define IOXX_INFO_MSG(chann)   BOOST_LOGL(::ioxx::logging::chann, info)
#define IOXX_WARN_MSG(chann)   BOOST_LOGL(::ioxx::logging::chann, warn)
#define IOXX_ERROR_MSG(chann)  BOOST_LOGL(::ioxx::logging::chann, err)
///@}

#endif // IOXX_LOGGING_HPP_INCLUDED
