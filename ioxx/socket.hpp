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

#ifndef IOXX_SOCKET_HPP_INCLUDED
#define IOXX_SOCKET_HPP_INCLUDED

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace ioxx
{
  class probe;                // necessary forward declaration

  class socket : private boost::noncopyable
  {
  public:
    typedef boost::shared_ptr<socket>   pointer;

    virtual ~socket() = 0;
    virtual void shutdown() = 0;

  protected:
    friend class probe;

    virtual bool input_blocked()  = 0;
    virtual bool output_blocked() = 0;

    virtual void unblock_input()  = 0;
    virtual void unblock_output() = 0;
    virtual void unblock_input_and_output() { unblock_input(); unblock_output(); }
  };

} // namespace ioxx

#endif // IOXX_SOCKET_HPP_INCLUDED
