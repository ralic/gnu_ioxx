/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.7 $
 * $Date: 2000/09/11 10:25:08 $
 *
 * Copyright (c) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include "scheduler.hpp"

class my_handler : public scheduler::event_handler
    {
  public:
    my_handler(scheduler& sched) : mysched(sched)
	{
	}
    ~my_handler()
	{
	}
    scheduler::handler_properties prop;
  private:
    virtual void fd_is_readable(int fd)
	{
	int rc = read(fd, &tmp, sizeof(tmp));
	if (rc == 0)
	    mysched.remove_handler(0);
	else if (rc > 0)
	    {
	    buffer.append(tmp, rc);
	    prop.poll_events = POLLOUT;
	    mysched.register_handler(1, *this, prop);
	    }
	}
    virtual void fd_is_writable(int fd)
	{
	int rc = write(fd, buffer.data(), buffer.length());
	if (rc > 0)
	    buffer.erase(0, rc);
	if (buffer.empty())
	    mysched.remove_handler(1);
	}
    virtual void read_timeout(int fd)
	{
	cout << "fd " << fd << " had a read timeout." << endl;
	}
    virtual void write_timeout(int fd)
	{
	cout << "fd " << fd << " had a write timeout." << endl;
	}
    char tmp[1024];
    string buffer;
    scheduler& mysched;
    };

int main()
try
    {
    scheduler sched;
    my_handler my_handler(sched);
    my_handler.prop.poll_events = POLLIN;
    sched.register_handler(0, my_handler, my_handler.prop);
    while (!sched.empty())
	sched.schedule();

    // done
    return 0;
    }
catch(const exception &e)
    {
    cerr << "Caught exception: " << e.what() << endl;
    return 1;
    }
catch(...)
    {
    cerr << "Caught unknown exception." << endl;
    return 1;
    }
