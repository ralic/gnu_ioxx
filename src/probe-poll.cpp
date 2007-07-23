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

#include "ioxx/probe.hpp"
#include "ioxx/type/system-error.hpp"
#include "ioxx/config.hpp"
#include <vector>
#include <map>
#include <boost/assert.hpp>
#include <boost/compatibility/cpp_c_headers/cerrno>
#include <sys/poll.h>           // XPG4-UNIX
#include "hot-fd.hpp"

// Abstract access to our logger.
#define TRACE_SOCKET(s, msg)     IOXX_TRACE_MSG("socket " << s << ": " << msg)

namespace {                     // don't export the following code

using std::size_t;
using ioxx::weak_socket;
using ioxx::hot_fd;
using ioxx::socket;

struct Poll : public ioxx::socket::probe
{
  struct context
  {
    socket::pointer     _f;             // shared_ptr to callback
    size_t              _pfd_index;     // index of this handler's pollfd?

    context(socket::pointer const & f, size_t i) : _f(f), _pfd_index(i)
    {
      BOOST_ASSERT(_f);
    }
  };

  typedef std::vector<pollfd>           pfd_array;
  typedef std::map<weak_socket,context> handler_set;
  typedef handler_set::iterator         iterator;
  typedef handler_set::const_iterator   const_iterator;
  typedef handler_set::value_type       pair_type;

  handler_set           _hset;
  pfd_array             _pfd;
  hot_fd                _hot_fd;

  static inline int const &    fd(const_iterator const & p)       { return p->first; }
  static inline size_t const & pfd_idx(const_iterator const & p)  { return p->second._pfd_index; }

  // Guarantee that an iterator is valid and points into consistent data.

  iterator const & check_consistency(iterator const & p) const
  {
    BOOST_ASSERT(p != _hset.end());
    BOOST_ASSERT(pfd_idx(p) <= _hset.size());
    BOOST_ASSERT(pfd_idx(p) <  _pfd.capacity());
    BOOST_ASSERT(fd(p) == _pfd[pfd_idx(p)].fd);
    return p;
  }

  // User access to our status.

  std::size_t   size()  const  { return _hset.size(); }
  bool          empty() const  { return _hset.empty(); }

  socket::pointer operator[] (weak_socket fd) const
  {
    const_iterator const p( _hset.find(fd) );
    return (p != _hset.end()) ? p->second._f : socket::pointer();
  }

  // Force a handler to be queried.

  void force(weak_socket s)
  {
    iterator const p( _hset.find(s) );
    if (p != _hset.end()) force(p);
    else                  TRACE_SOCKET(s, "ignore user-forced query");
  }

  void force(iterator const & p)
  {
    check_consistency(p);
    socket::pointer const & f( p->second._f );
    BOOST_ASSERT(f);
    _pfd[pfd_idx(p)].events
      = (f->input_blocked(fd(p))  ? POLLIN : 0)
      | (f->output_blocked(fd(p)) ? POLLOUT : 0)
      ;
    TRACE_SOCKET(fd(p), "requests probe event " << _pfd[pfd_idx(p)].events);
    BOOST_ASSERT(f);            // handler must still exist
  }

  // Append the new entry at the end of the pollfd array, then record
  // it in the handler set.

  void insert(weak_socket s, socket::pointer const & f)
  {
    TRACE_SOCKET(s, (f ? "insert" : "remove") << " handler");
    BOOST_ASSERT(s >= 0);
    iterator p( _hset.find(s) );
    if (p == _hset.end())       // add new entry
    {
      TRACE_SOCKET(s, "is not registered");
      if (!f) return;
      size_t const i( size() );
      TRACE_SOCKET(s, "insert at end of pfd array, " << i + 1u << " in set.");
      BOOST_ASSERT(i == _pfd.size());
      pollfd const pfd = { s, 0, 0};
      _pfd.push_back(pfd);
      try        { p = _hset.insert(pair_type(s, context(f, i))).first; }
      catch(...) { _pfd.resize(i); throw; }
      force(p);
    }
    else                        // overwrite existing entry
    {
      p->second._f = f;
      if (f)                    force(p);
      else if (!_hot_fd.hot(s)) erase(p);
    }
  }

  void erase(iterator const & p)
  {
    TRACE_SOCKET(fd(p), "erase context");
    check_consistency(p);
    BOOST_ASSERT(!_hot_fd.hot(fd(p)));
    size_t const i( pfd_idx(p) );
    size_t const last( size() - 1u );
    BOOST_ASSERT(last == _pfd.size() - 1u);
    if (i != last)
    {
      iterator const plast( _hset.find(_pfd[last].fd) );
      check_consistency(plast); // must exist
      TRACE_SOCKET(fd(p), "move socket " << plast->first << " in pfd array to erase");
      BOOST_ASSERT(pfd_idx(plast) == last);
      plast->second._pfd_index = i;
      _pfd[i] = _pfd[last];
    }
    _hset.erase(p); /// \todo An exception here leads to inconsistent containers.
    _pfd.resize(last);
  }

  // The event delivery loop.

  size_t run_once(int timeout)
  {
    BOOST_ASSERT(!empty());

    // Do the system call and handle errors.

    size_t nfds( size() );
    BOOST_ASSERT(nfds > 0);
    IOXX_TRACE_MSG("probing " << nfds << " sockets, " << timeout << " second timeout");
    int const rc = ::poll(&_pfd[0], nfds, (timeout < 0 ? -1 : timeout * 1000));
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
    IOXX_TRACE_MSG("probe found " << rc << " active sockets");

    // Deliver the events.

    for (iterator p( _hset.begin() ); nfds && p != _hset.end(); /**/)
    {
      check_consistency(p);
      short const revents( _pfd[pfd_idx(p)].revents );
      if (!revents) { ++p; continue; }
      TRACE_SOCKET(fd(p), "received probe event " << revents);
      --nfds;
      try
      {
        hot_fd::scope guard(_hot_fd, fd(p));
        socket::pointer const & f( p->second._f );
        BOOST_ASSERT(f);
        if (revents & ~(POLLIN | POLLOUT))
        {
          f->shutdown(*this, fd(p));
          goto inactive;
        }
        if (revents & POLLIN)
        {
          f->unblock_input(*this, fd(p));
          if (!f) goto inactive;
        }
        if (revents & POLLOUT);
        {
          f->unblock_output(*this, fd(p));
          if (!f) goto inactive;
        }
        force(p);
      }
      catch(std::exception const & e)
      {
        TRACE_SOCKET(fd(p), "*** error " << e.what());
        goto inactive;
      }
      catch(...)
      {
        TRACE_SOCKET(fd(p), "terminate");
        goto inactive;
      }
      ++p;
      continue;

    inactive:            // jump out of the hod_fd scope before calling erase
      erase(p++);
    }

    IOXX_TRACE_MSG(rc << " events delivered; " << size() << " sockets registered");
    return rc;
  }

  // Trivial construction and destruction.

  Poll()   { IOXX_TRACE_MSG("constructing poll(2) probe at " << this);  }
  ~Poll()  { IOXX_TRACE_MSG("shutting down poll(2) probe at " << this); }
};

} // end of anonymous namespace

// ----- Public Constructor Function ------------------------------------------

ioxx::socket::probe * ioxx::socket::probe::make()
{
  return new Poll;
}
