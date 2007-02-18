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

#include "ioxx/stream-buffer.hpp"
#include "ioxx/socket.hpp"
#include <boost/array.hpp>
#include <boost/concept_check.hpp>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

int main(int, char**)
{
#if 0
  boost::function_requires<
    boost::Mutable_RandomAccessContainerConcept<
      ioxx::streambuf< boost::array<char, 1024> >
    >
  >();

  boost::function_requires<
    boost::Mutable_RandomAccessContainerConcept<
      ioxx::streambuf< std::string >
    >
  >();

  // Test static buffer version.
  {
    typedef boost::array<char, 1024>    array_t;
    ioxx::streambuf<array_t>            iob;
    I(!iob.size());
    I(iob.empty());
    I(iob.capacity() == iob.max_size());

    char const msg[] = "hello world";
    iob.reset(msg, msg + sizeof(msg));
    I(iob.size() == sizeof(msg));
    I(!iob.empty());
    I(iob.capacity() == iob.max_size());
    iob.consume(1);
    I(iob.capacity() < iob.max_size());
  }

  // Test string version
  {
    ioxx::streambuf< std::string > iob;
    iob.base().reserve(1024u);
    cout << "iob.size() = " << iob.size() << endl;
    iob.base() += "hello world";
    iob.append(iob.base().size());
    cout << "iob.size() = " << iob.size() << endl;
    cout << "iob.capacity() = " << iob.capacity() << endl;
    cout << "iob.base().capacity() = " << iob.base().capacity() << endl;

    I(iob[0] == 'h');
    iob.consume(1u);
    I(iob[0] == 'e');
    iob.consume(1u);
    I(iob[0] == 'l');
    iob.consume(1u);
    I(iob[0] == 'l');
  }
#endif
  {
    ioxx::data_socket in( ioxx::posix_socket(::open("/etc/passwd", O_RDONLY)) );
    ioxx::data_socket out( ioxx::posix_socket(STDOUT_FILENO) );
    I(in); I(out);
#if 0
    char                        buf[1024];
    ioxx::stream_buffer<char>   inbuf( buf );
    while(in && out)
    {
      if (inbuf.full() && !inbuf.flush_gap())
        throw runtime_error("buffer overflow");
      else
        inbuf.read(in);
      if (!inbuf.empty())
        inbuf.write(out);
    }
#endif
  }
  return 0;
}
