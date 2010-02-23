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

#include <ioxx/detail/config.hpp>
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
#include <ioxx/time.hpp>
#include <ioxx/detail/adns.hpp>
#include <iterator>
#include <iostream>

namespace ioxx { typedef detail::adns<> dns; }

struct print
{
  void operator() (std::string const * str) const
  {
    std::cout << (str ? *str : "NULL") << std::endl;
  }

  void operator() (std::vector<std::string> const * vec) const
  {
    if (vec)
    {
      std::cout << "[ ";
      std::copy(vec->begin(), vec->end(), std::ostream_iterator<std::string>(std::cout, " "));
      std::cout << "]" << std::endl;
    }
    else
      std::cout << "NULL" << std::endl;
  }

  void operator() (std::vector< std::pair< std::string,std::vector<std::string> > > const * vec) const
  {
    if (vec)
    {
      for (ioxx::dns::mxname_list::const_iterator i( vec->begin() ); i != vec->end(); ++i)
      {
        std::cout << "MX " << i->first << " [ ";
        std::copy(i->second.begin(), i->second.end(), std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << "]" << std::endl;
      }
    }
    else
      std::cout << "NULL" << std::endl;
  }
};
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_dns_resolver )
{
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
  ioxx::time_of_day    now;
  ioxx::dns::schedule  schedule(now.current_time_t());
  ioxx::dns::dispatch  dispatch;
  ioxx::dns            dns(schedule, dispatch, now.current_timeval());

  dns.query_mx("cryp.to", print());
  dns.query_ptr("1.0.0.127.in-addr.arpa", print());
  dns.query_a("ecrc.de", print());
  for (;;)
  {
    dispatch.run();
    schedule.run();             // TODO: Bah! dns::cun() should return a second_t.
    dns.run();
    ioxx::seconds_t timeout( schedule.run() );
    if (schedule.empty())
    {
      if (dispatch.empty())  break;
      else                   timeout = dispatch.max_timeout();
    }
    dispatch.wait(timeout);
    now.update();
  }
#endif
}
