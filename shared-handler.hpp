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

#ifndef IOXX_SHARED_HANDLER_HPP_INCLUDED
#define IOXX_SHARED_HANDLER_HPP_INCLUDED

#include <boost/bind.hpp>
#include <boost/pointee.hpp>
#include "probe.hpp"

namespace ioxx
{
  /**
   *  \brief Register a pointed-to socket handler conveniently.
   */
  template <class PointerT>
  inline void add_shared_handler( probe &               probe
                                , socket const &        s
                                , event                 evmask
                                , PointerT const &      ptr
                                )
  {
    typedef typename boost::pointee<PointerT>::type handler;
    probe.add( s
             , evmask
             , boost::bind(&handler::operator(), ptr, _1, _2, _3)
             );
  }
}

#endif // IOXX_SHARED_HANDLER_HPP_INCLUDED
