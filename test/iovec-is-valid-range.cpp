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

#include "ioxx/iovec.hpp"
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

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( verify_that_iovec_is_a_valid_range )
{
  { ioxx::iovec       iov = { 0, 0 }; test_range_type(iov); }
  { ioxx::iovec const iov = { 0, 0 }; test_range_type(iov); }
}
