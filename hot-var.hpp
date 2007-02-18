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

#ifndef IOXX_HOT_VAR_HPP_INCLUDED
#define IOXX_HOT_VAR_HPP_INCLUDED

#include "sanity/invariant.hpp"

namespace ioxx
{
  /**
   *  \brief An assertion-checked mutable variable.
   *
   *  This class is a tool to enforce synchronized access to a value
   *  \c T. The container has two states: empty and hot. Accessing the
   *  container while it is empty violates an invariant. To make the
   *  container hot, assign it a value by instantiating \c
   *  hot_var<T>::scope. Now, the same value can be accessed through
   *  the \c hot_var and the \c scope object ... until the \c scope
   *  object falls, well, out of scope. In addition to providing
   *  exception safety, usage of this class allows a trivial upgrade
   *  to a mutex-controlled resource, should the need arise.
   *
   *  Passing an in-band \c invalid value as a template argument rules
   *  out non-primitive types for \c T.
   */
  template <class T, T invalid>
  class hot_var : private boost::noncopyable
  {
    T           _v;

  public:
    hot_var() : _v(invalid)                     { }
    ~hot_var()                                  { I(!hot()); }
    operator T const & () const                 { I(hot()); return _v; }
    bool hot()            const                 { return _v != invalid; }

    class scope
    {
      T &  _v;
      bool hot() const                          { return _v != invalid; }

    public:
      scope(hot_var & self, T v) : _v(self._v)  { I(!hot()); _v = v; I(hot()); }
      ~scope()                                  { I( hot()); _v = invalid; }
      operator T const & () const               { I( hot()); return _v; }
    };
  };

} // namespace ioxx

#endif // IOXX_HOT_VAR_HPP_INCLUDED
