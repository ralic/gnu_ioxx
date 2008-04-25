#ifndef IOXX_PROBE_POLL_IMPL_HPP_INCLUDED_2008_04_20
#define IOXX_PROBE_POLL_IMPL_HPP_INCLUDED_2008_04_20

#include "socket.hpp"
#include "socket-event.hpp"
#include <boost/noncopyable.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <sys/poll.h>           // XPG4-UNIX

namespace ioxx
{
  template < class VectorAllocator = std::allocator<pollfd>
           , class MapAllocator    = std::allocator< std::pair<socket_t const, typename std::vector<pollfd,VectorAllocator>::size_type> >
           >
  class probe_poll_implementation : private boost::noncopyable
  {
    typedef std::vector<pollfd,VectorAllocator>                                 pfd_array;
    typedef typename pfd_array::size_type                                       size_type;
    typedef std::map<socket_t,size_type,std::less<socket_t>,MapAllocator>       index_map;
    typedef typename index_map::iterator                                        iterator;

  public:
    probe_poll_implementation(unsigned int /* size_hint */) : _events(0u), _current(0u)
    {
    }

    void set(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      std::pair<iterator,bool> const r( _indices.insert(std::make_pair(s, _pfd.size())) );
      if (r.second)
      {                         // new entry
        try
        {
          pollfd const pfd = { s, 0, 0};
          _pfd.push_back(pfd);
        }
        catch(...)
        {
          _indices.erase(s);
          throw;
        }
      }
      pollfd & pfd( _pfd[r.first->second] );
      BOOST_ASSERT(s == pfd.fd);
      pfd.events = 0;
      if (request & ev_readable) pfd.events |= POLLIN;
      if (request & ev_writable) pfd.events |= POLLOUT;
      if (request & ev_pridata)  pfd.events |= POLLPRI;
    }

    void modify(socket_t s, socket_event request)
    {
      BOOST_ASSERT(s >= 0);
      set(s, request);
    }

    void unset(socket_t s)
    {
      BOOST_ASSERT(s >= 0);
      iterator const p( _indices.find(s) );
      if (p == _indices.end()) return;
      BOOST_ASSERT(_pfd.size());
      size_type i( p->second );
      size_type last( _pfd.size() - 1u);
      if (i != last)
      {
        iterator const plast( _indices.find(_pfd[last].fd) );
        BOOST_ASSERT(plast != _indices.end()); // must exist
        BOOST_ASSERT(plast->second == last);
        plast->second = i;
        _pfd[i] = _pfd[last];
      }
      _indices.erase(p); /// \todo An exception here leads to inconsistent containers.
      _pfd.resize(last);
    }

    static seconds_t max_timeout()
    {
      return static_cast<seconds_t>(std::numeric_limits<int>::max() / 1000);
    }

    bool pop_event(socket_t & s, socket_event & sev)
    {
      while (_events)
      {
        IOXX_TRACE_MSG("poll::pop_event() has " << _events << " events to deliver; _current = " << _current);
        pollfd const & pfd( _pfd[_current++] );
        s = pfd.fd;
        BOOST_ASSERT(s >= 0);
        sev = ev_idle;
        if (pfd.revents & POLLIN)  { sev |= ev_readable; }
        if (pfd.revents & POLLOUT) { sev |= ev_writable; }
        if (pfd.revents & POLLPRI) { sev |= ev_pridata;  }
        if (sev != ev_idle)
        {
          --_events;
          return true;
        }
      }
      return false;
    }

    void wait(seconds_t timeout)
    {
      BOOST_ASSERT(timeout <= max_timeout());
      BOOST_ASSERT(!_events);
      int const rc( poll(&_pfd[0], _pfd.size(), static_cast<int>(timeout) * 1000) );
      IOXX_TRACE_MSG("poll::wait() returned " << rc);
      if (rc < 0)
      {
        if (errno == EINTR) return;
        boost::system::system_error err(errno, boost::system::errno_ecat, "poll(2)");
        throw err;
      }
      _events  = static_cast<size_type>(rc);
      _current = 0u;
    }

  private:
    pfd_array _pfd;
    index_map _indices;
    size_type _events;
    size_type _current;
  };

} // namespace ioxx

#endif // IOXX_PROBE_POLL_IMPL_HPP_INCLUDED_2008_04_20
