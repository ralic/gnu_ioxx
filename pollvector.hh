/*
 * $Source: /home/cvs/lib/libscheduler/pollvector.hh,v $
 * $Revision: 1.8 $
 * $Date: 2001/09/15 16:41:11 $
 *
 * Copyright (c) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#ifndef __POLLVECTOR_HH__
#define __POLLVECTOR_HH__

// ISO C++ headers
#include <iostream>
#include <stdexcept>
#include <algorithm>

// POSIX system headers
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>

class pollvector
    {
  public:
    pollvector() : array(0), array_size(0), array_len(0)
	{
	reserve(1);
	}

    ~pollvector()
	{
	if (array)
	    free(array);
	}

    pollfd& operator[](int fd)
	{
	reserve(array_len+1);
	std::pair<pollfd*,pollfd*> i = std::equal_range(array, array+array_len, fd, pollfd_less());
	if (i.first - i.second > 1)
	    throw std::logic_error("scheduler::pollvector: The internal poll array is broken!");
	else if (i.first == i.second)
	    {			// We have to insert it.
	    memmove(i.first+1, i.first, (array+array_len-i.first)*sizeof(pollfd));
	    ++array_len;
	    i.first->fd      = fd;
	    i.first->events  = 0;
	    i.first->revents = 0;
	    return *i.first;
	    }
	else
	    return *i.first;	// Found it.
	}

    void erase(int fd)
	{
	std::pair<pollfd*,pollfd*> i = std::equal_range(array, array+array_len, fd, pollfd_less());
	if (i.first - i.second > 1)
	    throw std::logic_error("scheduler::pollvector: The interal poll array is broken!");
	else if (i.first == i.second)
	    return;
	else
	    {			// Found it.
	    memmove(i.first, i.first+1, (array+array_len-i.first-1)*sizeof(pollfd));
	    reserve(--array_len);
	    }
	}

    size_t length() const throw()
	{
	return array_len;
	}

    pollfd* get_pollfd_array() throw()
	{
	return array;
	}

    void dump(std::ostream& os) const
	{
	os << "The poll vector has " << array_len << " entries; size is " << array_size << "." << std::endl;
	for (size_t i = 0; i < array_len; ++i)
	    os << "fd = "        << std::dec << array[i].fd << "; "
	       << "events = 0x"  << std::hex << array[i].events << "; "
	       << "revents = 0x" << std::hex << array[i].revents
	       << std::dec << std::endl;

	}

  private:
    pollvector(const pollvector&); // Don't copy me.
    pollvector& operator=(const pollvector&);

    void reserve(size_t size)
	{
	size_t  new_size;
	pollfd* new_array;

	if (size <= array_len)
	    return;		// Bullshit.
#if 0
	else if (size < array_size/2 && array_size/2 > MIN_SIZE)
	    {			// Shrink array.
	    for (size_t n = array_size/2; n > size && n > MIN_SIZE; n /= 2)
		new_size = n;
	    debug("Shrinking pollvec array from %d to %d slots; we have %d entries.", array_size, new_size, array_len);
	    new_array = static_cast<pollfd*>(realloc(array, new_size*sizeof(pollfd)));
	    if (new_array == 0)
		return;		// How is this supposed to happen?
	    array      = new_array;
	    array_size = new_size;
	    }
#endif
	else if (size > array_size)
	    {			// Enlarge array.
	    for (new_size = (array_size > 0) ? array_size*2 : MIN_SIZE; new_size < size; new_size *= 2)
		;
	    new_array = static_cast<pollfd*>(realloc(array, new_size*sizeof(pollfd)));
	    if (new_array == 0)
		throw std::bad_alloc();
	    array      = new_array;
	    array_size = new_size;
	    }
	}

    static const size_t MIN_SIZE = 32;

    pollfd* array;
    size_t  array_size;
    size_t  array_len;

    struct pollfd_less
	{
        bool operator()(const pollfd& lhs, const int rhs) { return lhs.fd < rhs; }
        bool operator()(const int lhs, const pollfd& rhs) { return lhs < rhs.fd; }
	};
    };

#endif // !defined(__POLLVECTOR_HH__)
