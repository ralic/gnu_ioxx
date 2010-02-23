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

#include <ioxx/socket.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE( cannot_construct_invalid_system_socket )
{
  BOOST_REQUIRE_THROW(ioxx::system_socket(-1), std::invalid_argument);
}

///// New Socket Type /////////////////////////////////////////////////////////

typedef int native_socket_t;

template <class T, class SuperT = boost::noncopyable>
struct native_socket_type : public SuperT
{
  typedef T socket_type;

  friend inline void close(socket_type & s)
  {
    std::cout << __PRETTY_FUNCTION__ << " socket " << s << std::endl;
    ioxx::throw_errno_if_minus1("close(2)", boost::bind(boost::type<int>(), &::close, get_native_socket(s)));
  }

  friend inline bool valid(socket_type const & s)
  {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return get_native_socket(s) >= 0;
  }

  friend bool operator== (socket_type const & lhs, socket_type const & rhs)
  {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return get_native_socket(lhs) == get_native_socket(rhs);
  }
};

struct nil_t { };

class weak_socket : public native_socket_type<weak_socket, nil_t>
{
public:
  explicit weak_socket(native_socket_t s = -1) : _sock(s >= 0 ? s : -1) { }

  void reset(native_socket_t s = -1) { *this = weak_socket(s); }

private:
  native_socket_t       _sock;

  friend inline native_socket_t get_native_socket(weak_socket const & self)
  {
    return self._sock;
  }

  typedef void (weak_socket::*bool_type)(native_socket_t);

public:
  operator bool_type () const
  {
    return _sock >= 0 ? &weak_socket::reset : 0;
  }
};

class basic_socket : public native_socket_type<basic_socket>
{
public:
  explicit basic_socket(native_socket_t s = -1) : _sock(s >= 0 ? s : -1) { }

  ~basic_socket()
  {
    if (*this) close(*this);
  }

  void swap(basic_socket & other)    { std::swap(_sock, other._sock); }
  void reset(native_socket_t s = -1) { basic_socket(s).swap(*this); }
  native_socket_t release()          { native_socket_t s(_sock); _sock = -1; return s; }

private:
  native_socket_t       _sock;

  friend inline native_socket_t get_native_socket(basic_socket const & self)
  {
    return self._sock;
  }

  typedef native_socket_t (basic_socket::*bool_type)();

public:
  operator bool_type () const
  {
    return _sock >= 0 ? &basic_socket::release : 0;
  }
};

BOOST_AUTO_TEST_CASE( test_new_socket_type )
{
  basic_socket s, t;
  BOOST_REQUIRE_EQUAL(s, t);
  BOOST_REQUIRE_EQUAL(get_native_socket(s), -1);
  BOOST_REQUIRE(!valid(s));
  BOOST_REQUIRE(!s);
}

BOOST_AUTO_TEST_CASE( test_weak_socket_type )
{
  weak_socket s(-1), t(-1);
  BOOST_REQUIRE_EQUAL(s, t);
  BOOST_REQUIRE_EQUAL(get_native_socket(s), -1);
  BOOST_REQUIRE(!valid(s));
}
