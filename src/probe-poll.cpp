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
#include "sanity/system-error.hpp"
#include "logging.hpp"
#include "hot-var.hpp"
#include <vector>
#include <map>
#include <cerrno>
#include <sys/poll.h>           // XPG4-UNIX

// ----- Local Definitions and Namespace Configuration ------------------------

using namespace ioxx;
using std::size_t;

// Abstract access to our logger.
#define TRACE() IOXX_DEBUG_MSG(probe)
#define INFO()  IOXX_INFO_MSG(probe)
#define WARN()  IOXX_WARN_MSG(probe)
#define ERROR() IOXX_ERROR_MSG(probe)

// Continue with literal string or operator<<().
#define SOCKET_TRACE(s) TRACE() << "socket " << s << ": "
#define SOCKET_INFO(s)  INFO()  << "socket " << s << ": "
#define SOCKET_ERROR(s) ERROR() << "socket " << s << ": "

// ----- System Know-how ------------------------------------------------------

static event to_event(short revents)            // pollfd events to ioxx events
{
  return ((revents & POLLIN)              ? Readable : None)
       | ((revents & POLLOUT)             ? Writable : None)
       | ((revents & ~(POLLIN | POLLOUT)) ? Error    : None)
       ;
}

static short to_pollfd(event e)                 // ioxx event to pollfd events
{
  I(no_error(e));
  return short( (is_readable(e) ? POLLIN  : 0)
              | (is_writable(e) ? POLLOUT : 0)
              );
}

static pollfd to_pollfd(int fd, event evmask)   // pollfd constructor
{
  I(no_error(evmask));
  pollfd pfd;
  pfd.fd      = fd;
  pfd.events  = to_pollfd(evmask);
  pfd.revents = 0;
  return pfd;
}

// ----- Implementation of the probe API --------------------------------------

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
struct Poll : public probe
{
  struct context
  {
    socket const _s;
    handler      _f;
    size_t       _pfd_index;

    context(socket const & s, handler const & f, size_t i) : _s(s), _f(f), _pfd_index(i) { I(s); I(f); }
  };

  typedef std::vector<pollfd>           pfd_array;
  typedef std::map<int,context>         handler_set;
  typedef handler_set::iterator         iterator;
  typedef handler_set::value_type       pair_type;

  handler_set   _hset;
  pfd_array     _pfd;

  // Trivial construction and destruction.

  Poll()        { TRACE() << "constructing poll(2) probe at " << this;  }
  ~Poll()       { TRACE() << "shutting down poll(2) probe at " << this; }

  // Recognize the currently running handler.

  typedef ioxx::hot_var<int, -1>        hot_fd;
  hot_fd                                _hot_fd;
  bool is_hot(int fd) const             { return (_hot_fd.hot() && _hot_fd == fd); }

  // For convenience.

  void throw_overflow() const { throw overflow("poll()-based probe reached its maximum capacity"); }

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

  std::size_t   size()   const  { return _hset.size(); }
  bool          active() const  { return _hset.begin() != _hset.end(); }

  // Append the new entry at the end of the pollfd array, then record
  // it in the handler set.

  void add(socket const & s, event evmask, handler const & f)
  {
    SOCKET_INFO(s) "registering handler for " << evmask;
    I(s); I(f); I(no_error(evmask));
    int    const fd = s->fd;
    size_t const i  = size();
    SOCKET_TRACE(s) "assign pfd position " << i << "; have " << _pfd.size() << " in set.";
    I(i == _pfd.size());
    _pfd.push_back(to_pollfd(fd, evmask));
    iterator p = _hset.insert(pair_type(fd, context(s, f, i))).first;
    check_consistency(p);
  }

  // Find the context for the given socket, then use the recorded
  // index to update the pollfd array.

  void modify(socket const & s, event evmask)
  {
    SOCKET_INFO(s) "modifying event mask to " << evmask;
    I(s); I(no_error(evmask));
    I(!is_hot(s->fd));             // would be meaningless: return code wins
    size_t const i = lookup(s->fd)->second._pfd_index;
    _pfd[i].events = to_pollfd(evmask);
  }

  // We fill the gap in the pollfd array by copying the last entry
  // down.

  void del(iterator p)
  {
    check_consistency(p);
    int const fd = p->first;

    if (is_hot(fd))
    {
      SOCKET_INFO(fd) "Handler committed suicide.";
      _pfd[p->second._pfd_index].revents = POLLERR;
    }
    else
    {
      SOCKET_INFO(fd) "unregister handler";
      p->second._f = handler();         // run destructor
      size_t const i    = p->second._pfd_index;
      size_t const last = size() - 1; I(last == _pfd.size() - 1);
      if (i != last)
      {
        SOCKET_TRACE(p->first) "moving pfd entry " << last << " to index " << i << " to erase";
        iterator const plast = lookup(_pfd[last].fd);
        I(plast->second._pfd_index == last);
        plast->second._pfd_index = i;
        _pfd[i] = _pfd[last];
      }
      _pfd.resize(last);
      _hset.erase(p);
    }
  }

  // Trigger a handler, and delete it in case of an error. The
  // iterator has been advanced when trigger() returns.

  bool trigger(iterator & p, event e)
  {
    {
      size_t const        i   = p->second._pfd_index;
      pollfd &            pfd = _pfd[i];
      hot_fd::scope const fd(_hot_fd, pfd.fd);

      SOCKET_TRACE(fd) "triggering " << e;
      I(p->second._f);
      event evmask = p->second._f(p->second._s, e, *this);
      check_consistency(p);

      if (no_error(e) && no_error(evmask) && no_error(to_event(pfd.revents)))
      {
        size_t const j = p->second._pfd_index;
        I(i == j); /// todo Can the index change? A del() might have relocated us.
        _pfd[j].events = to_pollfd(evmask);
        ++p;
        return true;
      }
      p->second._f = handler();         // run destructor.
    }
    del(p++);
    return false;
  }

  // Public API forwards to functions above.

  bool trigger(socket const & s, event e)       { iterator p = lookup(s->fd); return trigger(p, e); }
  void del(socket const & s)                    { del(lookup(s->fd)); }

  // The event delivery loop.

  size_t run_once(msec_timeout_t timeout)
  {
    I(active()); I(timeout >= -1);

    // Do the system call and handle errors.

    size_t nfds = size();
    I(nfds > 0);
    INFO() << "probing " << nfds << " sockets, " << timeout << " msec timeout";
    int const rc = ::poll(&_pfd[0], nfds, timeout);
    if (rc < 0)
    {
      static char const id[] = "probe::run_once() failed";
      switch(errno)
      {
        case EINTR:     throw false;
        case EINVAL:    throw_overflow();
        case ENOMEM:    throw std::bad_alloc();
        default:        throw system_error(id);
      }
    }
    nfds = rc;
    TRACE() << nfds << " sockets are active";

    // Deliver the events.

    for (iterator p = _hset.begin(); nfds != 0 && p != _hset.end(); /**/)
    {
      check_consistency(p);
      short const revents = _pfd[p->second._pfd_index].revents;
      if (revents)
      {
        --nfds;
        trigger(p, to_event(revents)); // advances p for us
      }
      else
        ++p;
    }

    INFO() << rc << " events delivered; " << size() << " sockets waiting";
    return rc;
  }
};

// ----- Public Constructor Function ------------------------------------------

probe * ioxx::make_probe_poll()
{
  return new Poll;
}
