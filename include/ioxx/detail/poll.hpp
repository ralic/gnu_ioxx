/*
 * Copyright (c) 2008 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Copying and distribution of this file, with or without modification, are
 * permitted in any medium without royalty provided the copyright notice and
 * this notice are preserved.
 */

#ifndef IOXX_DETAIL_POLL_HPP_INCLUDED_2008_04_20
#define IOXX_DETAIL_POLL_HPP_INCLUDED_2008_04_20

#include "socket.hpp"
#include <boost/noncopyable.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <iosfwd>
#include <poll.h>

namespace ioxx { namespace detail
{
  typedef unsigned int seconds_t;

  template < class VectorAllocator = std::allocator<pollfd>
           , class MapAllocator    = std::allocator< std::pair<native_socket_t const, typename std::vector<pollfd,VectorAllocator>::size_type> >
           >
  class poll : private boost::noncopyable
  {
  public:
    typedef std::vector<pollfd,VectorAllocator>                                         pfd_array;
    typedef typename pfd_array::size_type                                               size_type;

    typedef std::map<native_socket_t,size_type,std::less<native_socket_t>,MapAllocator> index_map;
    typedef typename index_map::iterator                                                iterator;

    class socket : public detail::socket
    {
    public:
      enum event_set
        { no_events = 0
        , readable  = POLLIN
        , writable  = POLLOUT
        , pridata   = POLLPRI
        };

      friend inline event_set & operator|= (event_set & lhs, event_set rhs) { return lhs = (event_set)((int)(lhs) | (int)(rhs)); }
      friend inline event_set   operator|  (event_set   lhs, event_set rhs) { return lhs |= rhs; }
      friend inline event_set & operator&= (event_set & lhs, event_set rhs) { return lhs = (event_set)((int)(lhs) & (int)(rhs)); }
      friend inline event_set   operator&  (event_set   lhs, event_set rhs) { return lhs &= rhs; }
      friend inline std::ostream & operator<< (std::ostream & os, event_set ev)
      {
        if (ev == no_events) os << "None";
        if (ev & readable)   os << "Read";
        if (ev & writable)   os << "Write";
        if (ev & pridata)    os << "Pridata";
        return os;
      }

      socket(poll & demux, native_socket_t sock, event_set ev = no_events) : detail::socket(sock), _poll(demux)
      {
        BOOST_ASSERT(sock >= 0);
        std::pair<iterator,bool> const r( _poll._indices.insert(std::make_pair(sock, _poll._pfd.size())) );
        _iter = r.first;
        BOOST_ASSERT(r.second);
        try
        {
          pollfd const pfd = { sock, 0, 0};
          _poll._pfd.push_back(pfd);
          request(ev);
        }
        catch(...)
        {
          _poll._indices.erase(_iter);
          throw;
        }
      }

      ~socket()
      {
        check_consistency();
        BOOST_ASSERT(_poll._pfd.size());
        size_type const i( _iter->second );
        size_type const last( _poll._pfd.size() - 1u );
        if (i != last)
        {
          BOOST_ASSERT(i < last);
          iterator const plast( _poll._indices.find(_poll._pfd[last].fd) );
          BOOST_ASSERT(plast != _poll._indices.end()); // must exist
          BOOST_ASSERT(plast->second == last);
          plast->second = i;
          _poll._pfd[i] = _poll._pfd[last];
        }
        _poll._indices.erase(_iter); /// \todo An exception here leads to inconsistent containers.
        _poll._pfd.resize(last);
      }

      void request(event_set ev)
      {
        check_consistency();
        _poll._pfd[_iter->second].events = ev;
      }

    protected:
      poll & context() { return _poll; }

    private:
      poll &   _poll;
      iterator _iter;

      void check_consistency()
      {
        BOOST_ASSERT(as_native_socket_t() >= 0);
        BOOST_ASSERT(_iter != _poll._indices.end());
        BOOST_ASSERT(_iter->first == as_native_socket_t());
        BOOST_ASSERT(_iter->second < _poll._pfd.size());
        BOOST_ASSERT(_poll._pfd[_iter->second].fd == as_native_socket_t());
      }
    };

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000);
    }

    poll(unsigned int /* size_hint */ = 0u) : _n_events(0u), _current(0u)
    {
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, typename socket::event_set & ev)
    {
      while (_n_events)
      {
        IOXX_TRACE_MSG("poll::pop_event() has " << _n_events << " events to deliver; _current = " << _current);
        pollfd const & pfd( _pfd[_current++] );
        BOOST_ASSERT(pfd.fd >= 0);
        if (!pfd.revents) continue;
        --_n_events;
        sock = pfd.fd;
        ev   = static_cast<typename socket::event_set>(pfd.revents);
        return true;
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
      int const rc( ::poll(&_pfd[0], _pfd.size(), static_cast<int>(timeout) * 1000) );
      IOXX_TRACE_MSG("poll::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        system_error err(errno, "poll(2)");
        throw err;
      }
      _n_events  = static_cast<size_type>(rc);
      _current = 0u;
    }

  private:
    pfd_array _pfd;
    index_map _indices;
    size_type _n_events;
    size_type _current;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_POLL_HPP_INCLUDED_2008_04_20
