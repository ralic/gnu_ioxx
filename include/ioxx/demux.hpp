#ifndef IOXX_DEMUX_HPP_INCLUDED_2008_04_20
#define IOXX_DEMUX_HPP_INCLUDED_2008_04_20

#include "config.hpp"

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

#endif // IOXX_DEMUX_HPP_INCLUDED_2008_04_20
