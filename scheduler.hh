/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.5 $
 * $Date: 2000/08/31 09:54:57 $
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
    struct EventHandler
	{
	virtual ~EventHandler() = 0;
	virtual void readable(int) = 0;
	virtual void writable(int) = 0;
	};

    Scheduler()  { }
    ~Scheduler() { }

    void dump()
	{
	cerr << "Dumping contents of event_handlers and pollvec:" << endl;
	for (size_t i = 0; i < event_handlers.size(); ++i)
	    {
	    cerr << "fd: " << event_handlers[i].fd    << " / " << pollvec[i].fd << ", "
		 << "flags: " << event_handlers[i].flags << " / " << pollvec[i].events << endl;
	    }
	}

    void schedule()
	{
	while(!event_handlers.empty())
	    {
	    poll(pollvec.get_pollvec(), pollvec.size(), -1);
	    for (size_t i = 0; i < pollvec.size(); ++i)
		{
		if (pollvec[i].revents & POLLIN)
		    event_handlers[i].handler->readable(pollvec[i].fd);
		if (pollvec[i].revents & POLLOUT)
		    event_handlers[i].handler->writable(pollvec[i].fd);
		}
	    }
	}

    void set_handler(int fd, EventHandler* handler, short flags)
	{
	flags &= (POLLIN | POLLOUT);
	int pos;
	if (find_event_handler(fd, pos) == false)
	    {
	    if (handler == 0 || flags == 0)
		return;

	    pollvec.insert(pos);
	    try {
		event_handlers.insert(event_handlers.begin()+pos, HandlerContext());
		}
	    catch(...)
		{
		pollvec.erase(pos);
		throw;
		}
	    event_handlers[pos].fd      = fd;
	    event_handlers[pos].handler = handler;
	    event_handlers[pos].flags   = flags;
	    pollvec[pos].fd             = fd;
	    pollvec[pos].events         = flags;
	    pollvec[pos].revents        = 0;
	    }
	else
	    {
	    if (handler && flags)
		{
		event_handlers[pos].handler = handler;
		event_handlers[pos].flags   = flags;
		}
	    else
		{
		pollvec.erase(pos);
		event_handlers.erase(event_handlers.begin()+pos);
		}
	    }
	}

  private:			// don't copy me
    Scheduler(const Scheduler&);
    Scheduler& operator= (const Scheduler&);

  private:
    struct HandlerContext
	{
	int            fd;
	EventHandler*  handler;
	short          flags;
	};
    vector<HandlerContext> event_handlers;
    pollvec_t pollvec;

  private:
    struct less_cmp
	{
	bool operator() (const int lhs, const HandlerContext& rhs) const
	    { return (lhs < rhs.fd); }
	bool operator() (const HandlerContext& lhs, const int rhs) const
	    { return (lhs.fd < rhs); }
	};

    bool find_event_handler(int fd, int& pos)
	{
	typedef vector<HandlerContext>::iterator eh_iterator;
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
		throw logic_error("Scheduler: Internal list of event handlers is corrupt!");
	    pos = i.first - event_handlers.begin();
	    return true;
	    }
	}
    };

Scheduler::EventHandler::~EventHandler()
    {
    }

#endif // !defined(__SCHEDULER_HPP__)
