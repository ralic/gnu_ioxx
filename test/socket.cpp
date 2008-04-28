#include "ioxx/socket.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( cannot_construct_invalid_socket )
{
  BOOST_REQUIRE_THROW(ioxx::socket(-1), std::invalid_argument);
}
