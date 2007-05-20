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

#include <iostream>
#include <boost/log/log_impl.hpp>
#include <boost/log/functions.hpp>
#include "logging.hpp"
#include "sanity/platform.hpp"

SANITY_DLL_EXPORT BOOST_DEFINE_LOG(ioxx::logging::catchall, "ioxx.misc")
SANITY_DLL_EXPORT BOOST_DEFINE_LOG(ioxx::logging::memory,   "ioxx.mem")
SANITY_DLL_EXPORT BOOST_DEFINE_LOG(ioxx::logging::probe,    "ioxx.probe")

static void write_to_cerr(std::string const &, std::string const & msg)
{
  std::cerr << msg << std::endl;
}

SANITY_DLL_EXPORT void ioxx::logging::init_cerr()
{
  using namespace boost::logging;

  manipulate_logs("ioxx.*")
    .add_modifier(&prepend_prefix)
    .add_appender(&write_to_cerr);
  flush_log_cache();
}
