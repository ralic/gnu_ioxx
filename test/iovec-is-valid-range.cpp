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

#include <ioxx/iovec.hpp>
#include <boost/concept_check.hpp>

template <typename T>
void test_range_type(T & r)
{
  typedef typename boost::range_value<T>::type                  value_type;
  typedef typename boost::range_size<T>::type                   size_type;
  typedef typename boost::range_difference<T>::type             difference_type;
  typedef typename boost::range_iterator<T>::type               iterator;
  typedef typename boost::range_const_iterator<T>::type         const_iterator;
  typedef typename boost::range_reverse_iterator<T>::type       reverse_iterator;
  typedef typename boost::range_const_reverse_iterator<T>::type const_reverse_iterator;
  typedef typename boost::range_result_iterator<T>::type        result_iterator;
  typedef typename boost::sub_range<T>::type                    sub_range;

  boost::function_requires< boost::UnsignedIntegerConcept<size_type> >();
  boost::function_requires< boost::SignedIntegerConcept<difference_type> >();
  boost::function_requires< boost::BidirectionalIteratorConcept<const_iterator> >();
  boost::function_requires< boost::BidirectionalIteratorConcept<iterator> >();
  boost::function_requires< boost::BidirectionalIteratorConcept<result_iterator> >();

  boost::size(r);       boost::empty(r);
  boost::begin(r);      boost::const_begin(r);
  boost::rbegin(r);     boost::const_rbegin(r);
  boost::end(r);        boost::const_end(r);
  boost::rend(r);       boost::const_rend(r);
}

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( verify_that_iovec_is_a_valid_range )
{
  { ioxx::iovec       iov = { 0, 0 }; test_range_type(iov); }
  { ioxx::iovec const iov = { 0, 0 }; test_range_type(iov); }
}
