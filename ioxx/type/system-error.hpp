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

#ifndef IOXX_TYPE_SYSTEM_ERROR_HPP_INCLUDED
#define IOXX_TYPE_SYSTEM_ERROR_HPP_INCLUDED

#include <stdexcept>

namespace ioxx                  /// C++ API to the native I/O system.
{
  /// \brief System errors are nondescript and sudden.
  struct system_error : public std::runtime_error
  {
    system_error();
    explicit system_error(std::string const & context);
  };
}

#endif // IOXX_TYPE_SYSTEM_ERROR_HPP_INCLUDED
