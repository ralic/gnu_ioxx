/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_HPP_INCLUDED_2010_02_23
#define IOXX_HPP_INCLUDED_2010_02_23

#include <ioxx/acceptor.hpp>
#include <ioxx/core.hpp>
#include <ioxx/dispatch.hpp>
#include <ioxx/error.hpp>
#include <ioxx/iovec.hpp>
#include <ioxx/schedule.hpp>
#include <ioxx/signal.hpp>
#include <ioxx/socket.hpp>
#include <ioxx/time.hpp>

/**
 * \namespace ioxx
 * \brief Asynchronous I/O for C++.
 */

/**
 * \mainpage Asynchronous I/O for C++
 *
 * <hr>
 *
 * \section toc_sec Table of Contents
 *
 * - \ref intro_sec
 * - \ref download
 * - \ref updates
 * - \ref building
 * - \ref contact
 * - \ref licensing
 *
 * <hr>
 *
 * \section intro_sec What is ioxx?
 *
 * The ioxx library provides primitives for asynchronous I/O in C++. That kind
 * of thing is typically useful for people who would like to implement a highly
 * concurrent network service, i.e. an application that performs input/output
 * simultaneously on a great number of sockets. The library's main components
 * are a socket event dispatcher, a time event dispatcher, and an asynchronous
 * DNS resolver. There is also a class interface to socket programming that
 * offers those wonders of modern C++ such as exception-style error reporting,
 * transparent resource management, type-safety, and support for custom memory
 * allocation strategies. Ioxx is thread-safe in the sense that it is fully
 * re-entrant. The code runs on any POSIX-compliant operating system, most
 * notably Linux, NetBSD, Solaris, AIX, HP/UX, and Windows.
 *
 * Ioxx depends mandatorily on the excellent <a href="http://boost.org/">Boost
 * libraries</a>. The optional asynchronous DNS resolver depends on <a
 * href="http://www.chiark.greenend.org.uk/~ian/adns/">GNU ADNS</a> version 1.4
 * (or later). The reference documentation can be re-built with <a
 * href="http://www.doxygen.org/">doxygen</a>.
 *
 * <hr>
 *
 * \section download How to download the latest version
 *
 * - Stable release: <a href="http://download.savannah.nongnu.org/releases/ioxx/ioxx-1.0.tar.gz">ioxx-1.0.tar.gz</a>\n
 *
 * - Development snapshot: <a href="http://git.savannah.gnu.org/gitweb/?p=ioxx.git;a=snapshot;h=HEAD;sf=tgz">ioxx-HEAD.tar.gz</a>\n
 *   \n
 *   The most current development snapshot comes directly from the source code
 *   management repository, which can be <a
 *   href="http://git.savannah.gnu.org/gitweb/?p=ioxx.git">browsed online</a> to
 *   see the change history, etc. Users of the <a
 *   href="http://git.or.cz/">git</a> utility can also download that repository
 *   to their local machines:\n
 *   \n
 *   \verbatim git clone git://git.sv.gnu.org/ioxx.git \endverbatim
 *   \n
 *   Note that unlike the stable releases, development snapshots do not contain
 *   the whole zoo of build scripts necessary to compile the code -- like \c
 *   configure. Those scripts can be generated with the command
 *   <code>autoreconf -i</code> if <a
 *   href="http://www.gnu.org/software/autoconf/">Autoconf</a> 2.61 and <a
 *   href="http://sources.redhat.com/automake/">Automake</a> 1.10 (or later
 *   versions) are available.
 *
 * <hr>
 *
 * \section updates How to be notified of new releases
 *
 * If you would like to be notified whenever a new version becomes available,
 * subscribe to the %ioxx <a
 * href="http://git.savannah.gnu.org/gitweb/?p=ioxx.git;a=rss">RSS
 * feed</a> or <a
 * href="http://git.savannah.gnu.org/gitweb/?p=ioxx.git;a=atom">ATOM
 * log</a>.
 *
 * <hr>
 *
 * \section building How to configure, compile, and install this library
 *
 * For the impatient: <code>./configure && make check && make install</code>
 *
 * Ioxx is a header-only library that does not require any compilation -- with
 * the exception of the test suite. The standard Autoconf installation
 * procedure is described at great length in the distributed file \c INSTALL.
 * In addition to the usual standard options, ioxx can be configured with a
 * number of additional switches. If those options are left unspecified, ioxx
 * tries to auto-detect whether those features are available or not. Specifying
 * these options is usually unnecessary. A status summary will be output at the
 * end of the configure run.
 *
 * - <code>BOOST_SUFFIX</code>: Most systems allow linking the boost libraries
 *   under their normal names, i.e. the unit test framework can be linked as \c
 *   -lboost_unit_test_framework. Other systems, however, have those libraries
 *   installed with special suffixes that denote the compiler version used to
 *   build the library. etc. On such a system, it will be necessar to assign
 *   the variable on the \c configure command line, for example: \c
 *   BOOST_SUFFIX=-gcc34. The boost libraries required only when building the
 *   unit tests.
 *
 * - <code>--enable-epoll</code>, <code>--enable-epoll-pwait</code>: Enable
 *   support for the Linux-specific \c epoll family of system calls.
 *
 * - <code>--enable-poll</code>, <code>--enable-ppoll</code>: Enable support
 *   for the POSIX call \c poll() and/or the non-standard extension \c ppoll().
 *
 * - <code>--enable-select</code>, <code>--enable-pselect</code>: Enable
 *   support for the POSIX call \c select() and/or the non-standard extension
 *   \c pselect().
 *
 * - <code>--enable-adns</code>: Enable asynchronous DNS resolving with <a
 *   href="http://www.chiark.greenend.org.uk/~ian/adns/">GNU ADNS</a> version
 *   1.4 (or later). This might require additional \c -I flags in \c CPPFLAGS
 *   and \c -L flags in \c LDFLAGS.
 *
 * - <code>--enable-internal-docs</code>: When re-building the reference
 *   documentation, include classes that are documented as internal. The
 *   default is not to include those classes.
 *
 * - <code>--with-doxygen=PATH</code>, <code>--with-dot=PATH</code>,
 *   <code>--with-perl=PATH</code>: Use these flags to specify the absolute
 *   paths to the corresponding binaries. Alternatively, the variables \c
 *   DOXYGEN, \c DOT, and \c PERL can be assigned on the \c configure
 *   command-line. These options concern only the reference documentation
 *   generated with <a href="http://www.doxygen.org/">doxygen</a>.
 *
 * <hr>
 *
 * \section contact How to get help, submit patches, or report bugs
 *
 * Please submit bug reports or feature requests to the <a
 * href="http://savannah.nongnu.org/bugs/?func=additem&group=ioxx">ioxx bug
 * tracker</a>. The library is still fairly new and I'm particularly interested
 * in hearing about portability problems, i.e. platforms on which ioxx doesn't
 * compile or fails the test suite.
 *
 * <hr>
 *
 * \section licensing Licensing
 *
 * The ioxx library is distributed under the terms of the <a
 * href="http://www.gnu.org/licenses/">GNU Lesser General Public License</a>
 * version 3 or, at your option, any later version. Ioxx is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/**
 * \page inetd Example inet daemon
 *
 * \brief Multiplex various stream services in a single thread.
 *
 * The standard tool \c inetd offers services like daytime and echo, which
 * are re-implemented in this example program. Our mini-inetd binds those two
 * services to TCP ports 8080 and 8081 respectively, and accepts incoming
 * requests for about 5 seconds.
 *
 * \sa \ref daytime
 * \sa \ref echo
 *
 * \dontinclude inetd.cpp
 * \skip #include
 * \until io.wait
 * \line }
 * \line }
 */

/**
 * \page daytime Example daytime service
 *
 * \brief Serve the current time of day.
 *
 * \sa \ref inetd
 *
 * \dontinclude daytime.hpp
 * \skip #include
 * \until };
 */

/**
 * \page echo Example echo service
 *
 * \brief Echo all received data back to the peer.
 *
 * \sa \ref inetd
 *
 * \dontinclude echo.hpp
 * \skip #include
 * \until };
 */

#endif // IOXX_HPP_INCLUDED_2010_02_23
