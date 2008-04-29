#ifndef IOXX_RESOLVER_ADNS_HPP_INCLUDED_2008_04_20
#define IOXX_RESOLVER_ADNS_HPP_INCLUDED_2008_04_20

#include "ioxx/scheduler.hpp"
#include "ioxx/dispatch.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <adns.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef IOXX_HAVE_POLL
#  include <sys/poll.h>
#else
#  error "ADNS requires poll() support; select() isn't implemented yet."
#endif

namespace ioxx { namespace resolver
{
  inline adns_initflags & operator|= (adns_initflags & lhs, adns_initflags rhs) { return lhs = (adns_initflags)((int)(lhs) | (int)(rhs)); }
  inline adns_initflags   operator|  (adns_initflags   lhs, adns_initflags rhs) { return lhs |= rhs; }
  inline adns_initflags & operator&= (adns_initflags & lhs, adns_initflags rhs) { return lhs = (adns_initflags)((int)(lhs) & (int)(rhs)); }
  inline adns_initflags   operator&  (adns_initflags   lhs, adns_initflags rhs) { return lhs &= rhs; }

  class adns : private boost::noncopyable
  {
  public:
    typedef scheduler<>                                 scheduler;
    typedef dispatch<>                                  dispatch;

    typedef std::string                                 hostname;
    typedef std::vector<hostname>                       hostname_list;

    typedef std::string                                 hostaddr;
    typedef std::vector<hostaddr>                       hostaddr_list;

    typedef std::pair<hostname,hostaddr_list>           mxname;
    typedef std::vector<mxname>                         mxname_list;

    typedef boost::function1<void, hostaddr_list *>     a_handler;
    typedef boost::function1<void, mxname_list *>       mx_handler;
    typedef boost::function1<void, hostname *>          ptr_handler;

  public:
    adns(scheduler & sched, dispatch & disp, timeval const & now)
    : _scheduler(sched), _dispatch(disp), _now(now), _pfds(ADNS_POLLFDS_RECOMMENDED)
    {
      adns_initflags flags( adns_if_noautosys | adns_if_nosigpipe );
#ifndef NDEBUG
      flags |= adns_if_debug | adns_if_checkc_freq;
#endif
      throw_rc_if_not_zero(adns_init(&_state, flags, static_cast<FILE*>(0)), "cannot initialize adns");
      BOOST_ASSERT(_state);
    }

    ~adns()
    {
      BOOST_ASSERT(_state);
      _scheduler.cancel(_timeout, _now.tv_sec);
      adns_finish(_state);
    }

    void query_a(char const * owner, a_handler const & h)
    {
      IOXX_TRACE_MSG("adns requests A record for " << owner);
      BOOST_ASSERT(h);
      submit(owner, adns_r_a, 0, boost::bind(handleA, _1, h));
    }

    void query_a_no_cname(char const * owner, a_handler const & h)
    {
      IOXX_TRACE_MSG("adns requests A record for " << owner << " (no cname)");
      BOOST_ASSERT(h);
      submit(owner, adns_r_a, 0 | adns_qf_cname_forbid, boost::bind(handleA, _1, h));
    }

    void query_mx(char const * owner, mx_handler const & h)
    {
      IOXX_TRACE_MSG("adns requests MX record for " << owner);
      BOOST_ASSERT(h);
      submit(owner, adns_r_mx, 0, boost::bind(handleMX, _1, h));
    }

    void query_ptr(char const * owner, ptr_handler const & h)
    {
      IOXX_TRACE_MSG("adns requests PTR record for " << owner);
      BOOST_ASSERT(h);
      submit(owner, adns_r_ptr, 0, boost::bind(handlePTR, _1, h));
    }

    void update()
    {
      IOXX_TRACE_MSG(   "adns::update() has " << _qset.size() << " open queries and "
                    << _registered_sockets.size() << " registered sockets");
      _scheduler.cancel(_timeout, _now.tv_sec);
      if (_qset.empty()) return _registered_sockets.clear();

      // Determine the file descriptors we have to probe for.

      int timeout;
      int nfds( _pfds.size() );
      for (int rc( ERANGE ); rc == ERANGE; /**/)
      {
        timeout = -1;
        rc = adns_beforepoll(_state, &_pfds[0], &nfds, &timeout, &_now);
        switch(rc)
        {
          case ERANGE:        BOOST_ASSERT(nfds > 0); _pfds.resize(nfds); break;
          case 0:             break;
          default:            throw boost::system::system_error(rc, boost::system::errno_ecat, "adns_beforepoll()");
        }
      }
      BOOST_ASSERT(nfds >= 0);
      IOXX_TRACE_MSG("adns_beforepoll() returned " << nfds << " sockets; timeout = " << timeout);

      // Set new timeout.

      if (timeout == 0) adns_processtimeouts(_state, &_now);
      else if (timeout > 0)
      {
        seconds_t to( timeout / 1000 );
        if (timeout % 1000) ++to;
        _timeout = _scheduler.at(_now.tv_sec + to, boost::bind(&adns_processtimeouts, _state, &_now));
      }

      // Re-register the descriptors in dispatch.

      std::sort(&_pfds[0], &_pfds[nfds], less());
      pollfd const * req( &_pfds[0] );
      pollfd const * req_end( &_pfds[nfds] );
      socket_set::iterator reg( _registered_sockets.begin() );
      socket_set::iterator reg_end( _registered_sockets.end() );
      for (;;)
      {
        if (req == req_end)
        {
          IOXX_TRACE_MSG("no more requested fds; erase " << std::distance(reg, reg_end) << " registered sockets");
          _registered_sockets.erase(reg, reg_end);
          break;
        }
        else if (reg == reg_end)
        {
          IOXX_TRACE_MSG("no more registered fds; add " << req_end - req << " new ones sockets");
          std::for_each(req, req_end, boost::bind(&adns::register_fd, this, _1));
          break;
        }
        else if (req->fd < (*reg)->as_native_socket_t())
        {
          IOXX_TRACE_MSG("requested socket " << req->fd << " is new");
          register_fd(*req);
          ++req;
        }
        else if (req->fd == (*reg)->as_native_socket_t())
        {
          IOXX_TRACE_MSG("socket " << req->fd << " must be modified");
          (*reg)->request( req->events & POLLIN  ? socket::readable : socket::no_events
                         | req->events & POLLOUT ? socket::writable : socket::no_events
                         | req->events & POLLPRI ? socket::pridata  : socket::no_events
                         );
          ++req; ++reg;
        }
        else
        {
          BOOST_ASSERT(req->fd > (*reg)->as_native_socket_t());
          IOXX_TRACE_MSG("registered socket " << *reg << " is no longer required");
          _registered_sockets.erase(reg++);
        }
      }
    }

  private:
    scheduler &         _scheduler;
    dispatch &          _dispatch;
    adns_state          _state;
    timeval const &     _now;
    scheduler::task_id  _timeout;

    typedef boost::shared_ptr<adns_answer const>        answer;
    typedef boost::function1<void, answer>              callback;
    typedef std::map<adns_query,callback>               query_set;
    query_set           _qset;

    typedef dispatch::socket                            socket;
    typedef boost::shared_ptr<socket>                   shared_socket;
    struct less
    {
      bool operator() (pollfd const & lhs, pollfd const & rhs) const { return lhs.fd < rhs.fd; }
      bool operator() (shared_socket const & lhs, shared_socket const & rhs) const
      {
        return lhs->as_native_socket_t() < rhs->as_native_socket_t();
      }
    };
    typedef std::set<shared_socket,less>                socket_set;
    socket_set          _registered_sockets;
    std::vector<pollfd> _pfds;

    void check_consistency() const
    {
#ifndef NDEBUG
      adns_forallqueries_begin(_state);
      for (adns_query qid; /**/; /**/)
      {
        IOXX_TRACE_MSG("check that ADNS query " << qid << " has a registered handler");
        qid = adns_forallqueries_next(_state, 0);
        if (qid == 0) break;
        adns_checkconsistency(_state, qid);
        BOOST_ASSERT(_qset.find(qid) != _qset.end());
      }
#endif
    }

    void submit(char const * owner, adns_rrtype rrtype, int flags, callback const & f)
    {
      adns_query qid;
      throw_rc_if_not_zero( adns_submit(_state, owner, rrtype, static_cast<adns_queryflags>(flags), static_cast<FILE*>(0), &qid)
                          , "submit DNS query"
                          );
      _qset[qid] = f;
      check_consistency();
    }

    void register_fd(pollfd const & pfd)
    {
      IOXX_TRACE_MSG("register new adns socket " << pfd.fd);
      BOOST_ASSERT(pfd.fd >= 0);
      BOOST_ASSERT(pfd.events != 0);
      shared_socket s;
      socket::event_set const ev( pfd.events & POLLIN  ? socket::readable : socket::no_events
                                | pfd.events & POLLOUT ? socket::writable : socket::no_events
                                | pfd.events & POLLPRI ? socket::pridata  : socket::no_events
                                );
      BOOST_ASSERT(ev != socket::no_events);
      s.reset(new socket(_dispatch, pfd.fd, boost::bind(&adns::process_fd, this, pfd.fd, _1), ev));
      s->close_on_destruction(false);
      std::pair<socket_set::iterator, bool> const r( _registered_sockets.insert(s) );
      BOOST_ASSERT(r.second);
    }

    void process_fd(native_socket_t fd, socket::event_set ev)
    {
      IOXX_TRACE_SOCKET(fd, "process adns events " << ev);
      if (ev & socket::readable) throw_rc_if_not_zero(adns_processreadable(_state, fd, &_now), "adns_processreadable");
      if (ev & socket::writable) throw_rc_if_not_zero(adns_processwriteable(_state, fd, &_now), "adns_processwriteable");
      if (ev & socket::pridata)  throw_rc_if_not_zero(adns_processexceptional(_state, fd, &_now), "adns_processexceptional");

      IOXX_TRACE_MSG("deliver adns events");
      check_consistency();
      for (;;)
      {
        answer          ans;
        callback        f;
        adns_query      qid(0);
        adns_answer *   a(0);
        int const       rc( adns_check(_state, &qid, &a, 0) );
        IOXX_TRACE_MSG("adns_check() returned " << rc);
        switch (rc)
        {
          case EINTR:   continue;
          case ESRCH:   BOOST_ASSERT(_qset.empty());  return;
          case EAGAIN:  BOOST_ASSERT(!_qset.empty()); return;
          case 0:       break;
          default:      throw boost::system::system_error(rc, boost::system::errno_ecat, "adns_check()");
        }
        IOXX_TRACE_MSG("deliver ADNS query " << qid);
        BOOST_ASSERT(a);
        ans.reset(a, &::free);
        query_set::iterator const i( _qset.find(qid) );
        BOOST_ASSERT(i != _qset.end());
        std::swap(f, i->second);
        _qset.erase(i);
        f(ans);
      }
      check_consistency();
    }

    static void handleA(answer a, a_handler h)
    {
      BOOST_ASSERT(a->type == adns_r_a);
      hostaddr_list hs;
      switch (a->status)
      {
        case adns_s_ok:
          BOOST_ASSERT(a->nrrs > 0);
          for (int i(0); i != a->nrrs; ++i)
          {
            char const * str( inet_ntoa(a->rrs.inaddr[i]) );
            BOOST_ASSERT(str);
            hs.push_back(str);
          }
          h(&hs);
          break;

        case adns_s_nxdomain:
        case adns_s_nodata:
          BOOST_ASSERT(a->nrrs == 0);
          h(&hs);
          break;

        default:
          BOOST_ASSERT(a->nrrs == 0);
          h(0);
          break;
      }
    }

    static void handleMX(answer a, mx_handler h)
    {
      BOOST_ASSERT(a->type == adns_r_mx);
      mxname_list mxs;
      std::multimap<int,adns_rr_hostaddr const *> mxmap;
      switch (a->status)
      {
        case adns_s_ok:
          BOOST_ASSERT(a->nrrs > 0);
          for (int i(0); i != a->nrrs; ++i)
          {
            adns_rr_inthostaddr const & addr( a->rrs.inthostaddr[i] );
            mxmap.insert(std::make_pair(addr.i, &addr.ha));
          }
          for (std::multimap<int,adns_rr_hostaddr const *>::iterator i(mxmap.begin()); i != mxmap.end(); ++i)
          {
            adns_rr_hostaddr const & addr( *i->second );
            hostaddr_list ha;
            if (addr.naddrs > 0)
              for (int j(0); j != addr.naddrs; ++j)
                ha.push_back(inet_ntoa( addr.addrs[j].addr.inet.sin_addr ));
            mxs.push_back(std::make_pair(addr.host, ha));
          }
          h(&mxs);
          break;

        case adns_s_nxdomain:
        case adns_s_nodata:
          BOOST_ASSERT(a->nrrs == 0);
          h(&mxs);
          break;

        default:
          BOOST_ASSERT(a->nrrs == 0);
          h(0);
          break;
      }
    }

    static void handlePTR(answer a, ptr_handler h)
    {
      BOOST_ASSERT(a->type == adns_r_ptr);
      hostname ha;
      switch (a->status)
      {
        case adns_s_ok:
          BOOST_ASSERT(a->nrrs == 1);
          ha.assign(a->rrs.str[0]);
          h(&ha);
          break;

        case adns_s_nxdomain:
        case adns_s_nodata:
          BOOST_ASSERT(a->nrrs == 0);
          h(&ha);
          break;

        default:
          BOOST_ASSERT(a->nrrs == 0);
          h(0);
          break;
      }
    }

    static void throw_rc_if_not_zero(int rc, std::string const & ctx)
    {
      if (rc == 0) return;
      boost::system::system_error err(rc, boost::system::errno_ecat, ctx);
      throw err;
    }
  };

}} // namespace ioxx::resolver

#endif // IOXX_RESOLVER_ADNS_HPP_INCLUDED_2008_04_20
