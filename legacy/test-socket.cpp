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

#include "ioxx/socket.hpp"
using namespace ioxx;

int main(int, char**)
{
  socket s;

#if 0
  data_socket sout( posix_socket(1) );
  I(sout);

  char const msg[] = "hello world\n";
  sout.write(msg, msg + sizeof(msg) - 1);
#endif
  return 0;
}
