/*
 * Copyright (c) 2006-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_HOT_FD_HPP_INCLUDED
#define IOXX_HOT_FD_HPP_INCLUDED

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

namespace ioxx
{
  class hot_fd : private boost::noncopyable
  {
    weak_socket _fd;

  public:
    hot_fd() : _fd(-1)                    { }
    ~hot_fd()                             { BOOST_ASSERT(_fd == -1); }

    bool hot(weak_socket const & s) const { BOOST_ASSERT(s >= 0); return s == _fd; }

    class scope
    {
      weak_socket &  _fd;

    public:
      scope(hot_fd & self, weak_socket v) : _fd(self._fd)
      {
        BOOST_ASSERT(_fd == -1);
        BOOST_ASSERT(v >= 0);
        _fd = v;
      }
      ~scope()
      {
        BOOST_ASSERT(_fd >= 0);
        _fd = -1;
      }
    };
  };

} // namespace ioxx

#endif // IOXX_HOT_FD_HPP_INCLUDED
