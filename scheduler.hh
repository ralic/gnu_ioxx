/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.2 $
 * $Date: 2000/08/22 16:06:38 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>
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
	cout << "Reallocating pollvec: current size = " << pollvec_size
	     << ", new size = " << new_size << endl;
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
	virtual bool operator() (int) = 0;
	};

    Scheduler()  { }
    ~Scheduler() { }

    void set_read_handler(int fd, io_callback_t* handler)
	{
	set_handler(fd, handler, POLLIN);
	}

    void set_write_handler(int fd, io_callback_t* handler)
	{
	set_handler(fd, handler, POLLOUT);
	}

    void dump()
	{
	cout << "Dumping contents of event_handlers and pollvec:" << endl;
	for (size_t i = 0; i < event_handlers.size(); ++i)
	    {
	    cout << "fd : " << event_handlers[i].fd << " / " << pollvec[i].fd << ", "
		 << "readable : " << event_handlers[i].readable << " / " << ((pollvec[i].events & POLLIN) ? "POLLIN" : "no")  << ", "
		 << "writable : " << event_handlers[i].writable << " / " << ((pollvec[i].events & POLLOUT) ? "POLLOUT" : "no")  << endl;
	    }
	}

  private:			// don't copy me
    Scheduler(const Scheduler&);
    Scheduler& operator= (const Scheduler&);

  private:
    struct EventHandler
	{
	int fd;
	io_callback_t* readable;
	io_callback_t* writable;

	bool empty()
	    {
	    return (!readable && !writable);
	    }
	};
    vector<EventHandler> event_handlers;
    typedef vector<EventHandler>::iterator eh_iterator;
    pollvec_t pollvec;

  private:
    struct less_cmp
	{
	bool operator() (const int lhs, const EventHandler& rhs) const
	    { return (lhs < rhs.fd); }
	bool operator() (const EventHandler& lhs, const int rhs) const
	    { return (lhs.fd < rhs); }
	};

    bool find_event_handler(int fd, int& pos)
	{
	pair<eh_iterator,eh_iterator> i =
	    equal_range(event_handlers.begin(), event_handlers.end(), fd, less_cmp());
	if (i.first == i.second)
	    {
	    pos = i.first - event_handlers.begin();
	    return false;
	    }
	else
	    {
	    if (i.second - i.first != 1)
		throw logic_error("Scheduler: Internal list of event handlers is corrupted!");
	    pos = i.first - event_handlers.begin();
	    return true;
	    }
	}

    void set_handler(int fd, io_callback_t* handler, int flag)
	{
	int pos;
	if (find_event_handler(fd, pos) == false)
	    {
	    if (handler == 0)
		return;

	    pollvec.insert(pos);
	    try {
		event_handlers.insert(event_handlers.begin()+pos, EventHandler());
		}
	    catch(...)
		{
		pollvec.erase(pos);
		throw;
		}
	    event_handlers[pos].fd = fd;
	    if (flag == POLLIN)
		event_handlers[pos].readable = handler;
	    else
		event_handlers[pos].writable = handler;
	    pollvec[pos].fd = fd;
	    pollvec[pos].events = flag;
	    }
	else
	    {
	    if (handler)
		{
		if (flag == POLLIN)
		    event_handlers[pos].readable = handler;
		else
		    event_handlers[pos].writable = handler;
		pollvec[pos].events |= flag;
		}
	    else
		{
		if (flag == POLLIN)
		    event_handlers[pos].readable = 0;
		else
		    event_handlers[pos].writable = 0;
		pollvec[pos].events &= ~flag;
		if (event_handlers[pos].empty())
		    {
		    pollvec.erase(pos);
		    event_handlers.erase(event_handlers.begin()+pos);
		    }
		}
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
