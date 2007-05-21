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

#include "ioxx/probe.hpp"
#include <vector>
#include <map>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <sys/poll.h>           // XPG4-UNIX
#include "hot-fd.hpp"

// Abstract access to our logger.
#ifndef NDEBUG
#  include <iostream>
#  define TRACE(msg)           std::cerr << msg << std::endl;
#  define SOCKET_TRACE(s, msg) TRACE("socket " << s << ": " << msg)
#else
#  define TRACE(msg)           ((void)(0))
#  define SOCKET_TRACE(s, msg) ((void)(0))
#endif

namespace {                     // don't export the following code

using std::size_t;
using ioxx::weak_socket;
using ioxx::hot_fd;

/**
 *  \brief A probe implementation based on poll(2).
 *
 *  We manage two containers at once: (1) a set of registered handlers
 *  and (2) an array of pollfd structures. A socket handler's context
 *  tells us which pfd[] index is assigned to this socket. Contexts
 *  can be looked up by file descriptors. The state is managed as
 *  follows:
 *
 *    add        Appended handler's pollfd to the end of the
 *               array, then record the appropriate context in the
 *               handler set.
 *
 *    modify     Lookup handler context. Use the index to access the
 *               assigned pollfd, and update registered events.
 *
 *    del        Erase the pollfd by copying the last valid array
 *               entry down, into its position. Erase the entry from
 *               the set, and then update the pfd index context of the
 *               handler we copied down.
 *
 *    run_once   After poll() has returned, we iterate over the
 *               handler set and check the corresponding pollfd
 *               entries for events to deliver. Deliver them. In case
 *               of \c Error being delivered or returned, we erase the
 *               handler through del().
 */
struct Poll : public ioxx::probe
{
  struct context
  {
    weak_socket const   _s;
    socket::pointer     _f;
    size_t              _pfd_index;

    context(int s, socket::pointer const & f, size_t i) : _s(s), _f(f), _pfd_index(i)
    {
      BOOST_ASSERT(s >= 0);
      BOOST_ASSERT(f);
    }
  };

  typedef std::vector<pollfd>           pfd_array;
  typedef std::map<int,context>         handler_set;
  typedef handler_set::iterator         iterator;
  typedef handler_set::const_iterator   const_iterator;
  typedef handler_set::value_type       pair_type;

  handler_set           _hset;
  pfd_array             _pfd;
  hot_fd                _hot_fd;

  // Trivial construction and destruction.

  Poll()   { TRACE("constructing poll(2) probe at " << this);  }
  ~Poll()  { TRACE("shutting down poll(2) probe at " << this); }

  // Guarantee that an iterator is valid and points into consistent data.

  iterator const & check_consistency(iterator const & p) const
  {
    BOOST_ASSERT(p != _hset.end());
    BOOST_ASSERT(p->second._pfd_index <= _hset.size());
    BOOST_ASSERT(p->second._pfd_index <  _pfd.capacity());
    BOOST_ASSERT(p->first == _pfd[p->second._pfd_index].fd);
    return p;
  }

  // User access to our status.

  std::size_t   size()  const  { return _hset.size(); }
  bool          empty() const  { return _hset.empty(); }

  // Force a handler to be queried.

  void force(weak_socket const & s)
  {
    iterator const p( _hset.find(s) );
    if (p != _hset.end() && p->second._f) force(p);
  }

  void force(iterator const & p)
  {
    check_consistency(p);
    context const & ctx( p->second );
    BOOST_ASSERT(ctx._f);
    _pfd[ctx._pfd_index].events
      = (ctx._f->input_blocked(ctx._s)  ? POLLIN : 0)
      | (ctx._f->output_blocked(ctx._s) ? POLLOUT : 0)
      ;
    BOOST_ASSERT(ctx._f);       // handler must still exist
  }

  void erase(iterator p)
  {
    BOOST_ASSERT(!_hot_fd.hot(p->first));
    check_consistency(p);
    size_t const i( p->second._pfd_index );
    size_t const last( size() - 1u );
    BOOST_ASSERT(last == _pfd.size() - 1u);
    if (i != last)
    {
      SOCKET_TRACE(p->first, "moving pfd entry " << last << " to index " << i << " to erase");
      iterator const plast( _hset.find(_pfd[last].fd) );
      check_consistency(plast); // must exist
      BOOST_ASSERT(plast->second._pfd_index == last);
      plast->second._pfd_index = i;
      _pfd[i] = _pfd[last];
    }
    _pfd.resize(last);
    /// \todo An exception here leads to inconsistent containers.
    _hset.erase(p++);
  }

  // Append the new entry at the end of the pollfd array, then record
  // it in the handler set.

  void insert(weak_socket const & s, socket::pointer const & f)
  {
    BOOST_ASSERT(s >= 0);
    iterator p( _hset.find(s) );
    if (p == _hset.end())       // add new entry
    {
      if (!f) return;
      size_t const i( size() );
      SOCKET_TRACE(s, "assign pfd position " << i << "; have " << _pfd.size() << " in set.");
      BOOST_ASSERT(i == _pfd.size());
      pollfd const pfd = { s, 0, 0};
      _pfd.push_back(pfd);
      /// \todo If this call fails with an exception, the containers are
      ///       inconsistent.
      p = _hset.insert(pair_type(s, context(s, f, i))).first;
      force(p);
    }
    else                        // overwrite existing entry
    {
      p->second._f = f;
      if (f)                    force(p);
      else if (!_hot_fd.hot(s)) erase(p);
    }
  }

  // Lookup a shared pointer by socket.

  socket::pointer operator[] (weak_socket const & fd) const
  {
    const_iterator const p( _hset.find(fd) );
    return (p != _hset.end()) ? p->second._f : socket::pointer();
  }

  // The event delivery loop.

  size_t run_once(ioxx::msec_timeout_t timeout)
  {
    BOOST_ASSERT(!empty()); BOOST_ASSERT(timeout >= -1);

    // Do the system call and handle errors.

    size_t nfds( size() );
    BOOST_ASSERT(nfds > 0);
    TRACE("probing " << nfds << " sockets, " << timeout << " msec timeout");
    int const rc = ::poll(&_pfd[0], nfds, timeout);
    if (rc < 0)
    {
      static char const id[] = "probe::run_once() failed";
      switch(errno)
      {
#if 0                           /// \todo What to do in case of EINTR?
        case EINTR:     throw false;
#endif
        case ENOMEM:    throw std::bad_alloc();
        default:        throw ioxx::system_error(id);
      }
    }
    nfds = rc;
    TRACE(nfds << " sockets are active");

    // Deliver the events.

    for (iterator p = _hset.begin(); nfds != 0 && p != _hset.end(); /**/)
    {
      check_consistency(p);
      context &         ctx( p->second );
      socket::pointer & f( ctx._f );
      short const       revents( _pfd[ctx._pfd_index].revents );
      if (revents)
      {
        hot_fd::scope guard(_hot_fd, ctx._s);
        --nfds;
        if (revents & ~(POLLIN | POLLOUT)) f.reset();
        else
        {
          if (f && (revents & POLLIN))  f->unblock_input(*this, ctx._s);
          if (f && (revents & POLLOUT)) f->unblock_output(*this, ctx._s);
        }
        check_consistency(p);
      }
      if (f)                    // handler is still active
      {
        /// \todo This code queries every registered handler every time the
        /// loop is iterated. Is this wise? It makes the handler more
        /// responsive: they get a chance to run even when no i/o takes place.
        /// It is a certain overhead though.
        force(p++);
      }
      else                      // handler is inactive: remove it
        erase(p++);
    }

    TRACE(rc << " events delivered; " << size() << " sockets waiting");
    return rc;
  }
};

} // end of anonymous namespace

// ----- Public Constructor Function ------------------------------------------

ioxx::probe * ioxx::make_probe_poll()
{
  return new Poll;
}
