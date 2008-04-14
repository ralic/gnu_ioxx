/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
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

#include "type/weak-socket.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace ioxx
{
  /**
   *  \brief A probed socket.
   *
   *  Derive from this class to create a low-level I/O service that can be
   *  registered in ioxx::socket::probe.
   */
  struct socket : private boost::noncopyable
  {
    struct probe;

    /**
     *  \brief Sockets are generally stored in smart pointers.
     *
     *  Resource leaks are devastating in I/O code, particularly in
     *  long-living servers. The reference-counting overhead is well worth
     *  having transparent resource tracking in return. If it is necessary to
     *  register a non-head-allocated ioxx::socket::probe::socket, the shared pointer
     *  can be constructed with an empty deleter.
     */
    typedef boost::shared_ptr<socket>   pointer;

    /// \brief Derived class provides destructor.
    virtual ~socket() = 0;

    /**
     *  \brief Shall we probe for input?
     *
     *  ioxx::socket::probe invokes this method for every registered socket
     *  handler to determine whether input becoming available is a requested
     *  signal. The method is also invoked after state change event has been
     *  delivered.
     *
     *  \note A handler is not supposed to modify the probe in this method.
     */
    virtual bool input_blocked(weak_socket s) const = 0;

    /**
     *  \brief Shall we probe for output?
     *  \sa    ioxx::socket::input_blocked()
     */
    virtual bool output_blocked(weak_socket s) const = 0;

    /**
     *  \brief Input can be read without blocking.
     *
     *  If the last call to ioxx::socket::input_blocked() has returned true,
     *  ioxx::socket::probe will invoke this method once the associated socket
     *  has become readable.
     */
    virtual void unblock_input(probe & p, weak_socket s)  = 0;

    /**
     *  \brief Output can be written without blocking.
     *  \sa    ioxx::socket::unblock_input()
     */
    virtual void unblock_output(probe & p, weak_socket s) = 0;

    /**
     *  \brief The OS has invalidated the socket.
     *
     *  This method is invoked when ioxx::probe receives an error condition
     *  from the underlying system call.
     */
    virtual void shutdown(probe & p, weak_socket s) = 0;
  };


  /**
   *  \brief The system's socket event dispatcher.
   *
   *  Over its lifetime a ioxx::weak_socket alternates between four states:
   *  Input can be blocked or available and output can be blocked or available.
   *  There are special kinds of sockets that support only input or only
   *  output: IPC pipes or listener sockets, for example. As far as the event
   *  dispatcher is concerned, however, those sockets are special only insofar
   *  as that they don't receive certain events.
   *
   *  A ioxx::socket::probe signals those state changes for a registered by
   *  invoking an appropriate handler object derived from ioxx::probe::socket.
   *  The internals of the probe object are system-dependent and remain opaque.
   */
  class socket::probe : private boost::noncopyable
  {
  public:
    /**
     *  \brief Create a native probe.
     *
     *  \return Pointer to an instance of ioxx::probe. Failure is signaled using
     *          exceptions.
     *
     *  \throw  std::bad_alloc Insufficient memory.
     *
     *  \throw  ioxx::system_error Implementation reports failure for
     *          reasons hard to describe other than std::exception::what().
     */
    static probe * make();

    /// \brief System-dependent Implementation provides destructor.
    virtual ~probe() = 0;

    /**
     *  \brief Manipulate the set of probed sockets.
     *
     *  Add \c s to set of probed sockets using \c p as its handler. If \c p is
     *  invalid, the socket will be \em removed from the set instead. When
     *  insert() is called for a socket that is already registered, it's
     *  handler will be replaced by \c p (if \c p is valid).
     *
     *  After inserting/replacing a handler, ioxx::probe::force() is invoked to
     *  determine the events it is interested in.
     *
     *  Adding, replacing, or deleting a handler is always a safe operation. It
     *  is possible for the currently running handler to modify its own entry.
     *  However, changes are immediate. If a handler replaces itself in the
     *  probe, it effectively performs <code>delete this</code> (unless another
     *  copy of the shared pointer exists somewhere else).
     *
     *  \param s  Socket to probe.
     *  \param p  Pointer to socket handler for \c s.
     */
    virtual void insert(weak_socket s, socket::pointer const & p) = 0;

    /// \brief A simple forwarder to ioxx::probe::insert().
    void remove(weak_socket s) { insert(s, socket::pointer()); }

    /**
     *  \brief Refresh the events this socket is probed for.
     *
     *  If \p s is a valid socket,  ioxx::probe::socket::input_blocked() and
     *  ioxx::probe::socket::output_blocked() will be invoked on its handler.
     */
    virtual void force(weak_socket) = 0;

    /// \brief  Obtain a copy of the shared pointer to a socket's handler.
    /// \return An invalid socket if \c s is not registered.
    virtual socket::pointer operator[] (weak_socket s) const = 0;

    /// \brief The number of registered socket in this probe.
    virtual std::size_t size()  const = 0;

    /// \brief Test whether a probe is empty.
    virtual bool        empty() const = 0;

    /**
     *  \brief Probe for new events and deliver them.
     *
     *  \param timeout  Block no longer than this many seconds.  0u => return
     *                  immediately; -1 => block indefinitely.
     *  \pre            <code>!empty()</code>
     *  \return         Number of sockets that had events delivered. \c 0u
     *                  signifies a timeout.
     */
    virtual std::size_t run_once(int timeout = -1) = 0;
  };
} // namespace ioxx

#endif // IOXX_PROBE_HPP_INCLUDED
