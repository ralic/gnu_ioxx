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

#ifndef IOXX_DETAIL_SIGNALS_HPP_INCLUDED_2008_04_20
#define IOXX_DETAIL_SIGNALS_HPP_INCLUDED_2008_04_20

#include <ioxx/detail/error.hpp>
#include <signal.h>

namespace ioxx { namespace detail
{
  class block_signals : private boost::noncopyable
  {
  public:
    block_signals()
    {
      sigset_t block_all;
      throw_errno_if_minus1("sigfillset(3)", boost::bind(&::sigfillset, &block_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(&::sigprocmask, SIG_SETMASK, &block_all, &_orig_mask));
    }

    ~block_signals()
    {
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(&::sigprocmask, SIG_SETMASK, &_orig_mask, static_cast<sigset_t*>(0)));
    }

  private:
    sigset_t _orig_mask;
  };

  class unblock_signals : private boost::noncopyable
  {
  public:
    unblock_signals()
    {
      sigset_t unblock_all;
      throw_errno_if_minus1("sigemptyset(3)", boost::bind(&::sigemptyset, &unblock_all));
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(&::sigprocmask, SIG_SETMASK, &unblock_all, &_orig_mask));
    }

    ~unblock_signals()
    {
      throw_errno_if_minus1("sigprocmask(2)", boost::bind(&::sigprocmask, SIG_SETMASK, &_orig_mask, static_cast<sigset_t*>(0)));
    }

  private:
    sigset_t _orig_mask;
  };

}} // namespace ioxx::detail

#endif // IOXX_DETAIL_SIGNALS_HPP_INCLUDED_2008_04_20
