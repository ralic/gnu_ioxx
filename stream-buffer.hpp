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

#ifndef IOXX_STREAM_BUFFER_HPP_INCLUDED
#define IOXX_STREAM_BUFFER_HPP_INCLUDED

#include "memory.hpp"
#include "socket.hpp"

namespace ioxx
{
  template <class CharT>
  class stream_buffer : public boost::iterator_range<CharT *>
  {
    boost::iterator_range<CharT *>      _buf;

  public:
    typedef CharT                                       value_type;
    typedef value_type *                                iterator;
    typedef boost::iterator_range<iterator>             range_type;

    stream_buffer()                                     { }
    stream_buffer(range_type const & r)   : _buf(r)     { reset(); }
    stream_buffer(iterator b, iterator e) : _buf(b, e)  { reset(); }

    range_type &       data()       { return *this; }
    range_type const & data() const { return *this; }
    range_type const & buf()  const { return _buf; }

    bool empty() const  { return data().empty(); }
    bool full()  const  { return data().end() == buf().end(); }

    void reset()        { data() = range_type(buf().begin(), buf().begin()); }
    bool flush_gap()    { return move_down(data(), buf().begin()); }
  };

  inline bool read(data_socket & s, stream_buffer<char> & sbuf)
  {
    I(s); I(!sbuf.full());
    char * new_data_end( s.read(sbuf.data().end(), sbuf.buf().end()) );
    if (new_data_end)
    {
      reset_end(sbuf.data(), new_data_end);
      // \todo We ignore blocking; this breaks the code in edge trigger mode.
      return true;
    }
    else
      return false;
  }

} // namespace ioxx

#endif // IOXX_STREAM_BUFFER_HPP_INCLUDED
