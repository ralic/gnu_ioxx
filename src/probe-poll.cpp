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
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <sys/poll.h>           // XPG4-UNIX

// Invariants short and easy.
#include <boost/assert.hpp>
#define I(exp) BOOST_ASSERT(exp)

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
    ioxx::system::socket const  _s;
    ioxx::socket::pointer       _f;
    size_t                      _pfd_index;

    context(int s, ioxx::socket::pointer const & f, size_t i) : _s(s), _f(f), _pfd_index(i)
    {
      I(s >= 0);
      I(f);
    }
  };

  typedef std::vector<pollfd>           pfd_array;
  typedef std::map<int,context>         handler_set;
  typedef handler_set::iterator         iterator;
  typedef handler_set::value_type       pair_type;

  handler_set   _hset;
  pfd_array     _pfd;

  // Trivial construction and destruction.

  Poll()        { TRACE("constructing poll(2) probe at " << this);  }
  ~Poll()       { TRACE("shutting down poll(2) probe at " << this); }

  // For convenience.

  iterator check_consistency(iterator p) const
  {
    I(p != _hset.end());
    I(p->second._pfd_index <= _hset.size());
    I(p->second._pfd_index <  _pfd.capacity());
    I(p->first == _pfd[p->second._pfd_index].fd);
    return p;
  }

  iterator lookup(int fd)               // cannot fail
  {
    return check_consistency(_hset.find(fd));
  }

  // User access to our status.

  std::size_t   size()  const  { return _hset.size(); }
  bool          empty() const  { return _hset.empty(); }

  // Append the new entry at the end of the pollfd array, then record
  // it in the handler set.

  void insert(ioxx::system::socket const & s, ioxx::socket::pointer const & f)
  {
    I(s >= 0);
    iterator p = _hset.find(s);
    if (p != _hset.end())  p->second._f = f;
    else
    {
      if (!f) return;
      size_t const i( size() );
      SOCKET_TRACE(s, "assign pfd position " << i << "; have " << _pfd.size() << " in set.");
      I(i == _pfd.size());
      pollfd pfd = { s, 0, 0 };
      pfd.events |= f->input_blocked()  ? POLLIN  : 0;
      pfd.events |= f->output_blocked() ? POLLOUT : 0;
      _pfd.push_back(pfd);
      /// \todo If this call fails with an exception, the containers are
      /// inconsistent.
      p = _hset.insert(pair_type(s, context(s, f, i))).first;
      check_consistency(p);
    }
  }

  // The event delivery loop.

  size_t run_once(ioxx::msec_timeout_t timeout)
  {
    I(!empty()); I(timeout >= -1);

    // Do the system call and handle errors.

    size_t nfds( size() );
    I(nfds > 0);
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
      context &                 ctx( p->second );
      ioxx::socket::pointer &   f( ctx._f );
      short const &             revents( _pfd[ctx._pfd_index].revents );
      if (revents)
      {
        --nfds;
        if (f)
        {
          if (revents & POLLIN)
          {
            if (revents & POLLOUT)      f->unblock_input_and_output();
            else                        f->unblock_input();
          }
          else
          {
            if (revents & POLLOUT)      f->unblock_output();
            else                        f.reset();
          }
          check_consistency(p);
        }
      }
      if (f)                    // handler is still active
      {
        /// \todo This code queries every registered handler every time the
        /// loop is iterated. Is this wise? It makes the handler more
        /// responsive: they get a chance to run even when no i/o takes place.
        /// It is a certain overhead though.
        _pfd[ctx._pfd_index].events
          = f->input_blocked()  ? POLLIN  : 0
          | f->output_blocked() ? POLLOUT : 0
          ;
        ++p;
      }
      else                      // handler is inactive: remove it
      {
        size_t const i( ctx._pfd_index );
        size_t const last( size() - 1u );
        I(last == _pfd.size() - 1u);
        if (i != last)
        {
          SOCKET_TRACE(ctx._s, "moving pfd entry " << last << " to index " << i << " to erase");
          iterator const plast( lookup(_pfd[last].fd) );
          I(plast->second._pfd_index == last);
          plast->second._pfd_index = i;
          _pfd[i] = _pfd[last];
        }
        _pfd.resize(last);
        /// \todo An exception here leads to inconsistent containers.
        _hset.erase(p++);
      }
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
