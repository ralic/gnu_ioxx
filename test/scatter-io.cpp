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
#include <vector>

int main(int, char**)
{
  ioxx::weak_socket             s( STDIN_FILENO );
  std::vector<ioxx::byte_type>  buffer(1024);
  std::vector<ioxx::iovec>      iovec_array;

  return 0;
}
