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

#ifndef IOXX_HPP_INCLUDED_2008_04_14
#define IOXX_HPP_INCLUDED_2008_04_14

#include <ioxx/acceptor.hpp>
#include <ioxx/core.hpp>
#include <ioxx/dispatch.hpp>
#include <ioxx/error.hpp>
#include <ioxx/iovec.hpp>
#include <ioxx/schedule.hpp>
#include <ioxx/signals.hpp>
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
 * The ioxx library provides primitives for asynchronous i/o in C++. That kind
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
 * <hr>
 *
 * \section download How to download the latest version
 *
 * - Stable release: <a href="http://ioxx.cryp.to/ioxx-0.1.tar.gz">ioxx-0.1.tar.gz</a>\n
 *   \n
 *   Ioxx depends mandatorily on the excellent <a
 *   href="http://boost.org/">Boost libraries</a>. The optional asynchronous
 *   DNS resolver depends on <a
 *   href="http://www.chiark.greenend.org.uk/~ian/adns/">GNU ADNS</a> version
 *   1.4 (or later). Optional logging support is enabled if the <a
 *   href="http://logxx.cryp.to/">logxx</a> library is available. The reference
 *   documentation can be re-built with <a
 *   href="http://www.doxygen.org/">doxygen</a>.
 *
 * - Development snapshot: <a href="http://git.cryp.to/?p=ioxx;a=snapshot;h=HEAD;sf=tgz">ioxx-HEAD.tar.gz</a>\n
 *   \n
 *   The most current development snapshot comes directly from the source code
 *   management repository, which can be <a
 *   href="http://git.cryp.to/?p=ioxx">browsed online</a> to see the change
 *   history, etc. Users of the <a href="http://git.or.cz/">git</a> utility can
 *   also download that repository to their local machines:\n
 *   \n
 *   \verbatim git clone http://ioxx.cryp.to/.git \endverbatim
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
 * subscribe to the %ioxx <a href="http://git.cryp.to/?p=ioxx;a=rss">RSS
 * feed</a> or <a href="http://git.cryp.to/?p=ioxx;a=atom">ATOM log</a>.
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
 * - <code>--enable-logging</code>: Enable support the <a
 *   href="http://logxx.cryp.to/">logxx</a> library. This might require
 *   additional \c -I flags in \c CPPFLAGS to help the compiler finding the \c
 *   logxx.hpp header. If this feature is disabled, ioxx won't produce any log
 *   file.
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
 * Please direct any correspondence to me <a href="mailto:simons@cryp.to">via
 * e-mail</a>. The library is still fairly new and I'm particularly interested
 * in hearing about portability problems, i.e. platforms on which ioxx doesn't
 * compile or fails the test suite. Patches and suggestions for improvement are
 * always welcome, of course.
 *
 * <hr>
 *
 * \section licensing Licensing
 *
 * The ioxx library is distributed under the terms of the <a
 * href="http://www.gnu.org/licenses/">GNU Lesser General Public License</a>
 * version 3 or, at your option, any later version. This choice is mostly a
 * consequence of ioxx's dependency on GNU ADNS, which dictates those terms.
 * However, only a fraction of the code distributed as part of ioxx actually
 * depends on ADNS, meaning that most files include the =adns.h= header neither
 * directly nor indirectly. When used on their own, these files are available
 * under an all-permissive license, which means that you can do with them,
 * basically, whatever you want.
 *
 * Ioxx is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 */

#endif // IOXX_HPP_INCLUDED_2008_04_14
