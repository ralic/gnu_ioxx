/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.10 $
 * $Date: 2000/08/17 13:58:28 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include <hash_map>
#include <time.h>
#include <sys/poll.h>
#include <algorithm>

class Scheduler
    {
  public:
    struct io_callback_t
	{
	virtual ~io_callback_t() = 0;
	virtual bool operator() (int) = 0;
	};
    struct to_callback_t
	{
	virtual ~to_callback_t() = 0;
	virtual bool operator() (time_t) = 0;
	};

  public:
    Scheduler() : pollvec(0), pollvec_size(0), pollvec_valid(false)
	{
	}
    ~Scheduler()
	{
	}

    void register_event_handler(int fd,
				io_callback_t* readable = 0, to_callback_t* readable_timeout = 0, int read_timeout = -1,
				io_callback_t* writable = 0, to_callback_t* writable_timeout = 0, int write_timeout = -1,
				to_callback_t* alarm = 0)
	{
	EventHandler& eh(event_handler_map[fd]);
	eh.fd               = fd;
	eh.readable         = readable;
	eh.readable_timeout = readable_timeout;
	eh.read_timeout     = read_timeout;
	eh.writable         = writable;
	eh.writable_timeout = writable_timeout;
	eh.write_timeout    = write_timeout;
	eh.alarm            = alarm;

	reserve(event_handler_map.size());
	pollvec_valid = false;
	}

    void schedule()
	{
	if (event_handler_map.empty())
	    return;
	else
	    build_pollvec();

	}

  private:			// don't copy me
    Scheduler(const Scheduler&);
    Scheduler& operator= (const Scheduler&);

    struct EventHandler
	{
	int             fd;
	io_callback_t*  writable;
	to_callback_t*  writable_timeout;
	int             write_timeout;
	io_callback_t*  readable;
	to_callback_t*  readable_timeout;
	int             read_timeout;
	to_callback_t*  alarm;
	};
    typedef hash_map<int,EventHandler> event_handler_map_t;
    event_handler_map_t event_handler_map;

    pollfd* pollvec;
    size_t  pollvec_size;
    bool    pollvec_valid;
    static const size_t INITIAL_POLLVEC_SIZE = 32;

    void reserve(size_t n)
	{
	if (pollvec_size >= n)
	    return;

	size_t new_size = (pollvec_size == 0) ? INITIAL_POLLVEC_SIZE : pollvec_size * 2;
	while (pollvec_size >= n)
	    new_size *= 2;
	pollfd* new_vec = (pollfd*)realloc(pollvec, new_size*sizeof(pollfd));
	if (new_vec == 0)
	    throw bad_alloc();

	pollvec      = new_vec;
	pollvec_size = new_size;
	}

    void build_pollvec()
	{
	if (pollvec_valid)
	    return;

	event_handler_map_t::const_iterator src = event_handler_map.begin();
	pollfd*                             dst = pollvec;
	while(src != event_handler_map.end())
	    {
	    dst->fd      = src->second.fd;
	    dst->events  = ((src->second.readable != 0) ? POLLIN : 0)
		         | ((src->second.writable != 0) ? POLLOUT : 0);
	    dst->revents = 0;
	    ++src;
	    ++dst;
	    }
	}
    };

Scheduler::io_callback_t::~io_callback_t()
    {
    }
Scheduler::to_callback_t::~to_callback_t()
    {
    }

#endif // !defined(__SCHEDULER_HPP__)
