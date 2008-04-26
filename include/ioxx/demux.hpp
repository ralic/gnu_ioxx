#ifndef IOXX_DEMUX_HPP_INCLUDED_2008_04_20
#define IOXX_DEMUX_HPP_INCLUDED_2008_04_20

#if defined(IOXX_HAVE_EPOLL)
#  include "demux/epoll.hpp"
namespace ioxx { typedef demux::epoll default_demux; }
#elif defined(IOXX_HAVE_POLL)
#  include "demux/poll.hpp"
namespace ioxx { typedef demux::poll<> default_demux; }
#elif defined(IOXX_HAVE_SELECT)
#  include "demux/select.hpp"
namespace ioxx { typedef demux::select default_demux; }
#else
#  error "No I/O de-multiplexer available for this platform."
#endif

namespace ioxx { namespace demux
{
  template <class T>
  struct demux_concept
  {
    void constraints()
    {
    }
  };

  class demux_archetype
  {
  public:
  };

}} // namespace ioxx::demux

#endif // IOXX_DEMUX_HPP_INCLUDED_2008_04_20
