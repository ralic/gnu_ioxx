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

#include "socket.hpp"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace ioxx
{
  class probe;                // necessary forward declaration

  /**
   *  \brief Events you can probe for.
   */
  enum event
    { None      = 1 << 0
    , Readable  = 1 << 1
    , Writable  = 1 << 2
    , Error     = 1 << 6
    , UNUSED    = 1 << 7      // all bits above the 8th are free
    };

  ///@{
  /**
   *  \brief Type-safe operations for \c event.
   */
  inline event & operator|= (event & ev, event e)  { return (ev = event(ev | int(e))); }
  inline event & operator&= (event & ev, event e)  { return (ev = event(ev & int(e))); }
  inline event   operator|  (event lhs, event rhs) { return (lhs |= rhs); }
  inline event   operator&  (event lhs, event rhs) { return (lhs &= rhs); }
  inline event   operator~  (event ev)             { return event(~int(ev)); }
  ///@}

  ///@{
  /**
   *  \brief Query an \c event bitmask.
   */
  inline bool is_error(event e)       { return (e & Error); }
  inline bool no_error(event e)       { return !is_error(e); }
  inline bool is_readable(event e)    { return (e & Readable); }
  inline bool is_writable(event e)    { return (e & Writable); }
  ///@}

  /**
   *  \brief Pretty-print \c event.
   */
  inline std::ostream & operator<< (std::ostream & os, event ev)
  {
    os << "{ ";
    if (is_readable(ev)) os << "read " ;
    if (is_writable(ev)) os << "write " ;
    if (is_error(ev))    os << "error " ;
    os << "}";
    return os;
  }

  /**
   *  \brief Callback type for probing sockets.
   *
   *  An I/O callback, confusingly referred to as socket handler
   *  henceforth, is any functor or function with the given
   *  signature. By registering such a function in a \c probe, you
   *  can subscribe to a set of socket events. \c probe calls the
   *  handler function every time one of the registered events
   *  occurs on the socket. A handler can change its subscription
   *  mask by returning the \c evmask. It can also register or
   *  unregister other handlers in the probe.
   *
   *  Exceptions a handler might throw propagate up to the caller of
   *  of \c probe::run_once(). You can interrupt the probe. It is
   *  probably wise not to do it, though, because there is no
   *  telling what odd system limitations your native \c probe
   *  implementation might have. Exceptions are notoriously
   *  unreliable across DSO boundaries or through plain C code (such
   *  as \c libevent).
   *
   *  \param s      Socket \c ev refers to.
   *  \param ev     Events that occurred.
   *  \param probe  The probe this handler is registered in.
   *  \return       The new event mask. Set \c Error to unregister.
   */
  typedef boost::function<event (socket const & s, event ev, probe & p)> handler;

  /**
   * \brief A timeout in milliseconds (<code>1/10^3</code>); \c -1
   *        means infinite.
   *
   * \note  Be careful when specifying long timeouts. If in doubt,
   *        check \c std::numeric_limits<msec_timeout_t>::max().
   */
  typedef int msec_timeout_t;

  /**
   *  \brief A socket event dispatcher.
   *
   *  Instantiate this class with a system-dependent factory
   *  function, e.g. \c make_probe_poll(). Once you have a probe,
   *  you can register socket handlers for one or several of the
   *  events defined in the \c event enumeration.
   *
   *  A probe will never \c close() any socket registered in it, not
   *  even in case of \c Error. This is entirely the handler's
   *  problem. The good news is that you can wrap \c
   *  boost::shared_ptr<> and other devilry in a \c handler
   *  callback; a handler will fall out of scope when unregistered.
   */
  class SANITY_DLL_EXPORT probe : private boost::noncopyable
  {
  public:
    /**
     *  \brief Derived object provides destructor.
     */
    virtual ~probe() = 0;

    /**
     *  \brief Thrown by \c add() when maximum capacity is reached.
     */
    struct SANITY_DLL_EXPORT overflow : public std::length_error
    {
      overflow(char const * msg);
    };

    /**
     *  \brief Register a new socket handler.
     *
     *  \pre   No handler is registered for this socket and
     *         <code>s && no_error(evmask) && f</code>.
     *
     *  \post  The socket handler is registered.
     *
     *  \throw probe::overflow if maximum capacity has been reached.
     *         When exactly this happens depends on the probe
     *         implementation, but it's save to assume the limit is
     *         high.
     */
    virtual void add(socket const & s, event evmask, handler const & f) = 0;

    /**
     *  \brief Change a socket handler's event subscription.
     *
     *  The currently running socket handler must \em not use this
     *  function to modify its own event mask. Doing so violates an
     *  invariant. It would be a O(log(n)) no-operation anyway,
     *  because the event mask the handler \em returns later takes
     *  precedence.
     *
     *  \pre   <code>s && no_error(evmask)</code> must hold, and \c s
     *         must be registered.
     */
    virtual void modify(socket const & s, event evmask) = 0;

    /**
     *  \brief Remove a socket (and its handler) from the probe.
     *
     *  Unregister a socket, and cause its handler to fall out of
     *  scope. A handler may call \c del() on its own socket instead
     *  of returning \c Error to terminate. It is safe to call \c
     *  del() in the handler's destructor.
     *
     *  \pre     The socket is valid and a handler is registered for it.
     *  \post    The handler is not registered.
     */
    virtual void del(socket const & s) = 0;

    /**
     *  \brief  Trigger a registered socket handler.
     *
     *  Triggering a handler may leads to its destruction. Handler
     *  writers who use this function beware: no recursion, do not
     *  trigger your own socket.
     *
     *  \pre    The socket is valid and a handler is registered for it.
     *  \return False if the handler has been unregistered, true otherwise.
     */
    virtual bool trigger(socket const & s, event e) = 0;

    /**
     *  \brief Return number of registered handlers.
     */
    virtual std::size_t size() const = 0;

    /**
     *  \brief Return false if the probe is empty or otherwise unusable.
     */
    virtual bool active() const = 0;

    /**
     *  \brief   Probe for new events, and deliver them.
     *
     *  \pre     <code>active() && timeout &gt;= -1</code>
     *  \return  Number of socket events we delivered. \c 0u
     *           signifies a timeout.
     */
    virtual std::size_t run_once(msec_timeout_t timeout = -1) = 0;
  };

  // ----- probe implementations --------------------------------------

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
   *
   *  \note   The \c run_once() method of this probe returns
   *          status codes \c success or \c interrupted; \c
   *          blocked is not used.
   */
  SANITY_DLL_EXPORT probe * make_probe_poll();

} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED
