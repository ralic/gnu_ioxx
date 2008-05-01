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

#ifndef IOXX_DEMUX_HPP_INCLUDED_2008_04_20
#define IOXX_DEMUX_HPP_INCLUDED_2008_04_20

#include "detail/config.hpp"

#if defined(IOXX_BUILDING_DOCUMENTATION)
namespace ioxx { typedef implementation_defined demux; }
#elif defined(IOXX_HAVE_EPOLL) && IOXX_HAVE_EPOLL
#  include "detail/epoll.hpp"
namespace ioxx { typedef detail::epoll demux; }
#elif defined(IOXX_HAVE_POLL) && IOXX_HAVE_POLL
#  include "detail/poll.hpp"
namespace ioxx { typedef detail::poll<> demux; }
#elif defined(IOXX_HAVE_SELECT) && IOXX_HAVE_SELECT
#  include "detail/select.hpp"
namespace ioxx { typedef detail::select demux; }
#else
#  error "No I/O de-multiplexer available for this platform."
#endif

#endif // IOXX_DEMUX_HPP_INCLUDED_2008_04_20
