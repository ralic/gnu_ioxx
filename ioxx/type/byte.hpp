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

#ifndef IOXX_TYPE_BYTE_HPP_INCLUDED
#define IOXX_TYPE_BYTE_HPP_INCLUDED

#include <boost/compatibility/cpp_c_headers/cstddef>

namespace ioxx
{
  /// \brief Bytes are signed or unsigned, we don't know.
  typedef char                                          byte_type;

  /// \brief An unsigned number of bytes.
  typedef std::size_t                                   byte_size;

  /// \brief A signed distance between two byte addresses.
  typedef std::ptrdiff_t                                byte_offset;
}

#endif // IOXX_TYPE_BYTE_HPP_INCLUDED
