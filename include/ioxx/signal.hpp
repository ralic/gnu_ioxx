/*
 * Copyright (c) 2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IOXX_SIGNALS_HPP_INCLUDED_2010_02_23
#define IOXX_SIGNALS_HPP_INCLUDED_2010_02_23

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

#endif // IOXX_SIGNALS_HPP_INCLUDED_2010_02_23
