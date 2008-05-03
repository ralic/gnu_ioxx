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

#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
#include <ioxx/time.hpp>
#include <ioxx/dns.hpp>
#include <iostream>

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

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_dns_resolver )
{
#if defined(IOXX_HAVE_ADNS) && IOXX_HAVE_ADNS
  ioxx::time           now;
  ioxx::dns::schedule  schedule;
  ioxx::dns::dispatch  dispatch;
  ioxx::dns            dns(schedule, dispatch, now.as_timeval());

  dns.query_mx("cryp.to", print());
  dns.query_ptr("1.0.0.127.in-addr.arpa", print());
  dns.query_a("ecrc.de", print());
  for (;;)
  {
    dispatch.run();
    dns.run();
    ioxx::seconds_t timeout( schedule.run(now.as_time_t()) );
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
