/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.3 $
 * $Date: 2000/08/22 17:45:14 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include <vector>
#include <algorithm>
#include <time.h>
#include "pollvec.hpp"

class Scheduler
    {
  public:
    struct io_callback_t
	{
	virtual ~io_callback_t() = 0;
	virtual void operator() (int) = 0;
	};
    struct to_callback_t
	{
	virtual ~to_callback_t() = 0;
	virtual void operator() (int, time_t) = 0;
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

    void schedule()
	{
	poll(pollvec, pollvec.size(), -1);
	for (size_t i = 0; i < pollvec.size(); ++i)
	    {
	    if (pollvec[i].revents & POLLIN && event_handlers[i].readable)
		{
		(*event_handlers[i].readable)(pollvec[i].fd);
		}
	    if (pollvec[i].revents & POLLOUT && event_handlers[i].writable)
		{
		(*event_handlers[i].writable)(pollvec[i].fd);
		}
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
