/*
 * $Source: /home/cvs/lib/libscheduler/pollvec.hpp,v $
 * $Revision: 1.1 $
 * $Date: 2000/08/22 18:59:31 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __POLLVEC_HPP__
#define __POLLVEC_HPP__

#include <new>
#include <stdexcept>
#include <string>
#include <sys/poll.h>

class pollvec_t
    {
  public:
    pollvec_t() : pollvec(0), pollvec_size(0), pollvec_len(0)
	{
	}
    ~pollvec_t()
	{
	if (pollvec)
	    free(pollvec);
	}
    void insert(size_t pos)
	{
	reserve(pollvec_len + 1);
	memmove(pollvec + pos + 1, pollvec + pos, (pollvec_len - pos) * sizeof(pollfd));
	pollvec_len += 1;
	}
    void erase(size_t pos)
	{
	reserve(pollvec_len - 1);
	memmove(pollvec + pos, pollvec + pos + 1, (pollvec_len - pos - 1) * sizeof(pollfd));
	pollvec_len -= 1;
	}
    pollfd& operator[] (const size_t pos)
	{
	if (pos >= pollvec_len)
	    throw out_of_range("Attempt to access pollvec beyond its contents.");
	return pollvec[pos];
	}
    operator pollfd* ()
	{
	return pollvec;
	}
    size_t size() const
	{
	return pollvec_len;
	}

  private:
    void reserve(size_t count)
	{
	if (count <= pollvec_size)
	    return;

	size_t new_size = (pollvec_size) ? pollvec_size * 2 : 32;
	while(new_size < count)
	    new_size *= 2;
	pollfd* new_vec  = (pollfd*)realloc(pollvec, new_size*sizeof(pollfd));
	if (!new_vec)
	    throw bad_alloc();

	pollvec      = new_vec;
	pollvec_size = new_size;
	}
    pollfd* pollvec;
    size_t  pollvec_size;
    size_t  pollvec_len;
    };

#endif // !defined(__POLLVEC_HPP__)
