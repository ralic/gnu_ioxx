/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.10 $
 * $Date: 2001/01/22 11:46:26 $
 *
 * Copyright (c) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

// ISO C++ headers
#include <stdexcept>
#include <map>

// POSIX system headers
#include <time.h>
#include <errno.h>

// My own headers
#include "pollvector.hpp"

class scheduler
    {
  public:
    class event_handler
	{
      public:
	event_handler() { }
	virtual ~event_handler() = 0;
	virtual void fd_is_readable(int) = 0;
	virtual void fd_is_writable(int) = 0;
	virtual void read_timeout(int) = 0;
	virtual void write_timeout(int) = 0;
	};

    struct handler_properties
	{
	short        poll_events;
	unsigned int read_timeout;
	unsigned int write_timeout;
	};

    void register_handler(int fd, event_handler& handler, const handler_properties& properties)
	{
	if (fd < 0)
	    throw invalid_argument("scheduler::register_handle(): File descriptors must be 0 or greater!");

	if (properties.poll_events == 0)
	    {
	    remove_handler(fd);
	    return;
	    }

	try
	    {
	    fd_context& fdc = registered_handlers[fd];
	    pollvec[fd].events = properties.poll_events;
	    fdc = properties;
	    fdc.handler = &handler;
	    time_t now = time(0);
	    if (properties.poll_events & POLLIN)
		{
		fdc.next_read_timeout = now + fdc.read_timeout;
		}
	    else
		fdc.next_read_timeout = 0;
	    if (properties.poll_events & POLLOUT)
		{
		fdc.next_write_timeout = now + fdc.write_timeout;
		}
	    else
		fdc.next_write_timeout = 0;
	    }
	catch(...)
	    {
	    remove_handler(fd);
	    }
	}

    void remove_handler(int fd)
	{
	if (fd < 0)
	    throw invalid_argument("scheduler::remove_handle(): File descriptors must be 0 or greater!");
	registered_handlers.erase(fd);
	pollvec.erase(fd);
	}

    const handler_properties* get_handler_properties(int fd)
	{
	if (fd < 0)
	    throw invalid_argument("scheduler::remove_handle(): File descriptors must be 0 or greater!");
	map<int,fd_context>::const_iterator i = registered_handlers.find(fd);
	if (i != registered_handlers.end())
	    return &(i->second);
	else
	    return 0;
	}

    void schedule()
	{
	// Do we have work to do at all?

	if (empty())
	    return;

	// Call poll(2).
      poll_it:
	int rc = poll(pollvec.get_pollfd_array(), pollvec.length(), -1);
	if (rc == -1)
	    {
	    if (errno == EINTR)
		goto poll_it;
	    else
		throw runtime_error("poll failed");
	    }
	time(&time_poll_returned);

	// Now deliver the callbacks.

	for (size_t i = 0; i < pollvec.length(); )
	    {
	    pollfd pfd = pollvec.get_pollfd_array()[i];
	    fd_context& fdc = registered_handlers[pfd.fd];

	    if (pfd.events & pfd.revents & POLLIN)
		{
		fdc.handler->fd_is_readable(pfd.fd);
		if (i >= pollvec.length() || pollvec.get_pollfd_array()[i].fd != pfd.fd)
		    continue;	// The handler was removed.
		if (fdc.next_read_timeout > 0)
		    fdc.next_read_timeout = time_poll_returned + fdc.read_timeout;
		}
	    if (pfd.events & pfd.revents & POLLOUT)
		{
		fdc.handler->fd_is_writable(pfd.fd);
		if (i >= pollvec.length() || pollvec.get_pollfd_array()[i].fd != pfd.fd)
		    continue;	// The handler was removed.
		if (fdc.next_write_timeout > 0)
		    fdc.next_write_timeout = time_poll_returned + fdc.write_timeout;
		}
	    ++i;
	    }
	}

    bool empty() const
	{
	return registered_handlers.empty();
	}

    void dump() const throw()
	{
	cout << "registered_handlers contains " << registered_handlers.size() << " entries." << endl;
	map<int,fd_context>::const_iterator i;
	for (i = registered_handlers.begin(); i != registered_handlers.end(); ++i)
	    {
	    cout << "fd = " << i->first;
	    if (i->second.poll_events & POLLIN)
		cout << "; readable (timeout: " << i->second.next_read_timeout << ")";
	    if (i->second.poll_events & POLLOUT)
		cout << "; writeable (timeout: " << i->second.next_write_timeout << ")";
	    cout << endl;
	    }
	}

  private:
    struct fd_context : public handler_properties
	{
	event_handler* handler;
	time_t next_read_timeout;
	time_t next_write_timeout;

	fd_context& operator= (const handler_properties& rhs)
	    {
	    this->handler_properties::operator=(rhs);
	    return *this;
	    }
	};
    map<int,fd_context> registered_handlers;
    pollvector pollvec;
    time_t time_poll_returned;
    };

// Destructors must exist, even if they're pure virtual.

scheduler::event_handler::~event_handler()
    {
    }

#endif // !defined(__SCHEDULER_HPP__)
