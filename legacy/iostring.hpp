/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_IOSTRING_HPP_INCLUDED
#define IOXX_IOSTRING_HPP_INCLUDED

#include <string>
#include "ioxx/iovector.hpp"

namespace ioxx
{
  /**
   *  \brief todo
   */
  template <class charT = char>
  struct iostring : public std::basic_string< iovector<charT> >
  {
    typedef charT                                       value_type;
    typedef std::size_t                                 size_type;

    typedef iovector<value_type>                        page_type;
    typedef typename page_type::iterator                page_iterator;
    typedef typename page_type::const_iterator          const_page_iterator;

    typedef std::basic_string<page_type>                vector_type;
    typedef typename vector_type::iterator              vector_iterator;
    typedef typename vector_type::const_iterator        const_vector_iterator;

    inline iostring() { }

    iostring & push_back(page_iterator begin, page_iterator end)
    {
      vector_type::push_back(page_type(begin, end));
      return *this;
    }

    operator bool () const { return !vector_type::empty(); }

    size_type total_size() const
    {
      size_type i( 0u );
      for (const_vector_iterator p(vector_type::begin()); p != vector_type::end(); ++p)
        i += p->size();
      return i;
    }
  };

} // ioxx

#endif // IOXX_IOSTRING_HPP_INCLUDED
