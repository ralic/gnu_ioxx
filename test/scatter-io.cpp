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

#include "ioxx/system.hpp"
#include <vector>

#include <sys/stat.h>           // open(2)
#include <fcntl.h>

int main(int, char**)
{
  ioxx::weak_socket            sin( open("/etc/profile", O_RDONLY) );
  if (sin < 0) throw ioxx::system_error("cannot open /etc/profile");
  ioxx::weak_socket             sout( STDOUT_FILENO );
  std::vector<ioxx::byte_type>  buffer(1024u);
  std::vector<ioxx::iovec>      iovec_array(1u);
  ioxx::reset(iovec_array[0], &buffer[0], &buffer[buffer.size()]);
  ioxx::read(sin, &iovec_array[0], &iovec_array[iovec_array.size()], 0, 0, "can't read");
  ioxx::write(sout, &iovec_array[0], &iovec_array[iovec_array.size()]);

  return 0;
}
