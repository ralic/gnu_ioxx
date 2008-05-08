#include <ioxx/socket.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( cannot_construct_invalid_system_socket )
{
  BOOST_REQUIRE_THROW(ioxx::system_socket(-1), std::invalid_argument);
}
