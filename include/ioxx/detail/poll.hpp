/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_DETAIL_POLL_HPP_INCLUDED_2010_02_23
#define IOXX_DETAIL_POLL_HPP_INCLUDED_2010_02_23

#include <ioxx/socket.hpp>
#include <ioxx/signal.hpp>
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

  /**
   * \internal
   *
   * \brief I/O demultiplexer implementation based on \c poll(2).
   *
   * \sa http://www.opengroup.org/onlinepubs/009695399/functions/poll.html
   */
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

    class socket : public system_socket
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

      socket(poll & demux, native_socket_t sock, event_set ev = no_events) : system_socket(sock), _poll(demux)
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
        LOGXX_TRACE("socket " << as_native_socket_t() << " requests events " << ev);
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

    poll() : _n_events(0u), _current(0u)
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.poll(" + detail::show(this) + ')');
    }

    bool empty() const { return _n_events == 0u; }

    bool pop_event(native_socket_t & sock, typename socket::event_set & ev)
    {
      while (_n_events)
      {
        LOGXX_TRACE("pop_event() has " << _n_events << " events to deliver; _current = " << _current);
        pollfd const & pfd( _pfd[_current++] );
        BOOST_ASSERT(pfd.fd >= 0);
        if (!pfd.revents) continue;
        --_n_events;
        sock = pfd.fd;
        ev   = static_cast<typename socket::event_set>(pfd.revents);
        ev  |= ev & POLLRDNORM ? socket::readable : socket::no_events; // weird, redundant extensions
        ev  |= ev & POLLRDBAND ? socket::pridata  : socket::no_events;
        ev  |= ev & POLLWRNORM ? socket::writable : socket::no_events;
        ev  &= socket::readable | socket::writable | socket::pridata;
        LOGXX_TRACE("deliver events " << ev << " on socket " << sock);
        return true;
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      LOGXX_TRACE("wait on " << _pfd.size() << " sockets for at most " << timeout << " seconds");
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_n_events);
#if defined(IOXX_HAVE_PPOLL) && IOXX_HAVE_PPOLL
      timespec const to = { timeout, 0 };
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
      int const rc( ::ppoll(&_pfd[0], _pfd.size(), &to, &unblock_all) );
#else
      int rc;
      {
        signal_unblock signal_scope;
        rc = ::poll(&_pfd[0], _pfd.size(), static_cast<int>(timeout) * 1000);
      }
#endif
      LOGXX_TRACE("wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        system_error err(errno, "poll(2)");
        throw err;
      }
      _n_events  = static_cast<size_type>(rc);
      _current = 0u;
    }

  protected:
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);

  private:
    pfd_array _pfd;
    index_map _indices;
    size_type _n_events;
    size_type _current;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_POLL_HPP_INCLUDED_2010_02_23
