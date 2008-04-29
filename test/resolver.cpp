#include "ioxx/timer.hpp"
#include "ioxx/resolver.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
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
    using namespace ioxx::resolver;
    if (vec)
    {
      for (adns::mxname_list::const_iterator i( vec->begin() ); i != vec->end(); ++i)
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

BOOST_AUTO_TEST_CASE( test_adns_resolver )
{
  ioxx::timer now;
  ioxx::resolver::adns::scheduler scheduler;
  ioxx::resolver::adns::dispatch dispatch;
  ioxx::resolver::adns resolver(scheduler, dispatch, now.as_timeval());

  resolver.query_mx("cryp.to", print());
  resolver.query_ptr("1.0.0.127.in-addr.arpa", print());
  resolver.query_a("ecrc.de", print());
  for (;;)
  {
    dispatch.run();
    resolver.run();
    ioxx::seconds_t timeout( scheduler.run(now.as_time_t()) );
    if (scheduler.empty())
    {
      if (dispatch.empty())  break;
      else                   timeout = dispatch.max_timeout();
    }
    dispatch.wait(timeout);
    now.update();
  }
}
