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

#ifndef IOXX_SIGNALS_HPP_INCLUDED_2008_05_19
#define IOXX_SIGNALS_HPP_INCLUDED_2008_05_19

#include <ioxx/error.hpp>
#include <signal.h>

namespace ioxx
{
  /**
   * Block all POSIX signals for the current scope.
   */
  class block_signals : private boost::noncopyable
  {
  public:
    block_signals()
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.signal");
      sigset_t block_all;
      throw_errno_if_minus1("sigfillset(3)", boost::bind(boost::type<int>(), &::sigfillset, &block_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &block_all, &_orig_mask));
      LOGXX_TRACE("block all");
    }

    ~block_signals()
    {
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &_orig_mask, static_cast<sigset_t*>(0)));
      LOGXX_TRACE("cancel block all");
    }

  private:
    sigset_t _orig_mask;
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);
  };

  /**
   * Unblock all POSIX signals for the current scope.
   */
  class unblock_signals : private boost::noncopyable
  {
  public:
    unblock_signals()
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.signal");
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &unblock_all, &_orig_mask));
      LOGXX_TRACE("unblock all");
    }

    ~unblock_signals()
    {
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &_orig_mask, static_cast<sigset_t*>(0)));
      LOGXX_TRACE("cancel unblock all");
    }

  private:
    sigset_t _orig_mask;
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);
  };

} // namespace ioxx

#endif // IOXX_SIGNALS_HPP_INCLUDED_2008_05_19
