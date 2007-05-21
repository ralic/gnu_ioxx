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

#ifndef IOXX_PROBE_HPP_INCLUDED
#define IOXX_PROBE_HPP_INCLUDED

#include "system.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace ioxx
{
  typedef int msec_timeout_t;

  class probe : private boost::noncopyable
  {
  public:

    struct socket : private boost::noncopyable
    {
      typedef boost::shared_ptr<socket>   pointer;

      virtual ~socket() = 0;

      virtual bool input_blocked()  const = 0;
      virtual bool output_blocked() const = 0;

      virtual void unblock_input(probe & p, weak_socket const & s)  = 0;
      virtual void unblock_output(probe & p, weak_socket const & s)  = 0;
      virtual void unblock_input_and_output(probe & p, weak_socket const & s)
      {
        unblock_input(p, s);
        unblock_output(p, s);
      }
    };

    virtual ~probe() = 0;

    virtual void insert(weak_socket const &, socket::pointer const &) = 0;
    void remove(weak_socket const & s) { insert(s, socket::pointer()); }

    // virtual socket::pointer operator[] (weak_socket const &) = 0;
    // virtual socket::pointer operator[] (socket const &) = 0;

    virtual std::size_t size()  const = 0;
    virtual bool        empty() const = 0;

    /**
     *  \brief   Probe for new events and deliver them.
     *  \return  Number of socket events we delivered. \c 0u
     *           signifies a timeout if \c empty() is false.
     */
    virtual std::size_t run_once(msec_timeout_t timeout = -1) = 0;
  };

  /**
   *  \brief Create a probe based on XPG4-UNIX's \c poll(2).
   *
   *  \return \c NULL if unavailable on this system.
   *
   *  \throw  std::bad_alloc if there is insufficient memory.
   *
   *  \throw  probe::overflow if the maximum capacity of poll()
   *          has been reached: \c add() and \c run_once().
   *
   *  \throw  system_error if \c poll() reports failure for
   *          unspecified reasons: \c run_once().
   */
  probe * make_probe_poll();

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED
