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

#ifndef IOXX_SYSTEM_IOVEC_TRAITS_HPP_INCLUDED
#define IOXX_SYSTEM_IOVEC_TRAITS_HPP_INCLUDED

#ifndef IOXX_SYSTEM_HPP_INCLUDED
#  error "include ioxx/system.hpp instead of this file"
#endif

#define IOXX_SPECIALIZE_IOVEC_TRAITS(t, mv, cv)                                         \
  template<> struct range_ ## t<ioxx::system::iovec>       { typedef ioxx::mv type; };  \
  template<> struct range_ ## t<ioxx::system::iovec const> { typedef ioxx::cv type; }

namespace boost
{
  IOXX_SPECIALIZE_IOVEC_TRAITS(value,                   byte_type,                    byte_type const);
  IOXX_SPECIALIZE_IOVEC_TRAITS(size,                    byte_size,                    byte_size);
  IOXX_SPECIALIZE_IOVEC_TRAITS(difference,              byte_offset,                  byte_offset);
  IOXX_SPECIALIZE_IOVEC_TRAITS(iterator,                byte_iterator,                byte_const_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_iterator,          byte_const_iterator,          byte_const_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(reverse_iterator,        byte_reverse_iterator,        byte_const_reverse_iterator);
  IOXX_SPECIALIZE_IOVEC_TRAITS(const_reverse_iterator,  byte_const_reverse_iterator,  byte_const_reverse_iterator);
}

#undef IOXX_SPECIALIZE_IOVEC_TRAITS

#endif // IOXX_SYSTEM_IOVEC_TRAITS_HPP_INCLUDED
