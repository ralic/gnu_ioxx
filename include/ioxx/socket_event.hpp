#ifndef IOXX_SOCKET_EVENT_HPP_INCLUDED_2008_04_20
#define IOXX_SOCKET_EVENT_HPP_INCLUDED_2008_04_20

#include <iosfwd>

namespace ioxx
{
  enum socket_event
    { ev_idle      = 0
    , ev_readable  = 1 << 1
    , ev_writable  = 1 << 2
    , ev_pridata   = 1 << 3
    };

  inline socket_event & operator|= (socket_event & lhs, socket_event rhs) { return lhs = (socket_event)((int)(lhs) | (int)(rhs)); }
  inline socket_event   operator|  (socket_event   lhs, socket_event rhs) { return lhs |= rhs; }
  inline socket_event & operator&= (socket_event & lhs, socket_event rhs) { return lhs = (socket_event)((int)(lhs) & (int)(rhs)); }
  inline socket_event   operator&  (socket_event   lhs, socket_event rhs) { return lhs &= rhs; }

  inline std::ostream & operator<< (std::ostream & os, socket_event sev)
  {
    if (sev == ev_idle)    os << "Idle";
    if (sev & ev_readable) os << "Read";
    if (sev & ev_writable) os << "Write";
    if (sev & ev_pridata)  os << "Pridat";
    return os;
  }
} // namespace ioxx

#endif // IOXX_SOCKET_EVENT_HPP_INCLUDED_2008_04_20
