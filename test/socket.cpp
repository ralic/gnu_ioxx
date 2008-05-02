#include <ioxx/detail/socket.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( cannot_construct_invalid_socket )
{
  BOOST_REQUIRE_THROW(ioxx::detail::socket(-1), std::invalid_argument);
}
