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

#include "ioxx/os.hpp"
#include <cerrno>

static std::string str()
{
    using namespace std;
    return strerror(errno);
}

ioxx::system_error::system_error()
: std::runtime_error(str())
{
}

ioxx::system_error::system_error(std::string const & msg)
: std::runtime_error(msg + ": " + str())
{
}
