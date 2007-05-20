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

#ifndef IOXX_WRITER_HPP_INCLUDED
#define IOXX_WRITER_HPP_INCLUDED

#include <string>
#include "scoped_socket.hpp"
#include "iovector.hpp"

namespace ioxx
{
  class writer
  {
    scoped_socket                       _sock;

    typedef iovector<char>              iovec_type;
    std::basic_string<iovec_type>       ioarray_type;
    ioarray_type                        _iov;

    typedef std::string                 buffer_type;
    buffer_type                         _buf;
    char const *                        _base;

    typedef bool (writer::*unspecified_bool_type) () const;

  public:
    writer() : _base(0) { }

    void append(iovec_type const & iov)    { _iov.push_back(iov); }
    void append(std::string const & buf)   { append(buf.begin(), buf.end); }

    template <class char_iterator>
    void append(char_iterator begin, char_iterator end)
    {
      if (begin != end)
      {
        char const * const * old_end( _base + _buf.size() );
        _buf.push_back(begin, end);
        char const * const * new_end( _base + _buf.size() );
        append( iovector_type(old_end, new_end) );
      }
    }

    void reset()                                { *this = writer(); }
    bool empty() const                          { return _iov.empty(); }
    operator unspecified_bool_type () const     { return empty() ? 0 : &writer::empty; }

    bool operator() (socket s)
    {
      if (*this)
      {
        std::ssize_t const rc = ::writev(s.get(), _iov.data(), _iov.size());
        if (rc < 0)
        {
          if (errno == EAGAIN)  return false;
          else                  throw socket_error("reading", s);
        }



      }
      return *this;
    }

  private:
    struct fix_base
    {
      char const * const                _old_base;
      char const * const                _old_end;
      std::ptrdiff_t const              _offset;

      fix_base(writer const & ctx) : _old_base ( ctx._base                   )
                                   , _old_end  ( _old_base + ctx_buf.size()  )
                                   , _offset   ( ctx._buf.data() - _old_base )
      {
      }

      void operator(iovec_type & iov) const
      {
        char const * const begin( iov.begin() );
        char const * const end( iov.end() );
        if (_old_base <= begin && _old_end <= end)
          iov = iovec_type(begin + _offset, end + _offset);
      }

    };

    void relocate_iov()
    {
      std::for_each(_iov.begin(), _iov.end(), fix_base(*this));
      _base = _buf.data();
    }
  };

} // namespace ioxx

#endif // IOXX_WRITER_HPP_INCLUDED
