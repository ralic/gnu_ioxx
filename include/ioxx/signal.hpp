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
   *
   * \sa signal_unblock
   */
  class signal_block : private boost::noncopyable
  {
  public:
    signal_block()
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.signal");
      sigset_t block_all;
      throw_errno_if_minus1("sigfillset(3)", boost::bind(boost::type<int>(), &::sigfillset, &block_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &block_all, &_orig_mask));
      LOGXX_TRACE("block all");
    }

    ~signal_block()
    {
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &_orig_mask, static_cast<sigset_t*>(0)));
      LOGXX_TRACE("cancel block all");
    }

  private:
    sigset_t _orig_mask;
    LOGXX_DEFINE_TARGET(LOGXX_SCOPE_NAME);
  };

  /**
   * \internal
   *
   * \brief Unblock all POSIX signals for the current scope.
   *
   * \sa signal_block
   */
  class signal_unblock : private boost::noncopyable
  {
  public:
    signal_unblock()
    {
      LOGXX_GET_TARGET(LOGXX_SCOPE_NAME, "ioxx.signal");
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(boost::type<int>(), &::sigemptyset, &unblock_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(boost::type<int>(), &::sigprocmask, SIG_SETMASK, &unblock_all, &_orig_mask));
      LOGXX_TRACE("unblock all");
    }

    ~signal_unblock()
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
