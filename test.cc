/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.10 $
 * $Date: 2001/01/22 14:23:32 $
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
	if (buffer.empty())
	    mysched.remove_handler(1);
	int rc = write(fd, buffer.data(), buffer.length());
	if (rc > 0)
	    buffer.erase(0, rc);
	}
    virtual void read_timeout(int fd)
	{
	std::cerr << "fd " << fd << " had a read timeout." << std::endl;
	}
    virtual void write_timeout(int fd)
	{
	std::cerr << "fd " << fd << " had a write timeout." << std::endl;
	}
    char tmp[1024];
    std::string buffer;
    scheduler& mysched;
    };

int main()
try
    {
    scheduler sched;
    my_handler my_handler(sched);
    my_handler.prop.poll_events   = POLLIN;
    my_handler.prop.read_timeout  = 5;
    my_handler.prop.write_timeout = 0;
    sched.register_handler(0, my_handler, my_handler.prop);
    while (!sched.empty())
	{
	sched.schedule();
	sched.dump(std::cerr);
	}

    // done
    return 0;
    }
catch(const std::exception &e)
    {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
    }
catch(...)
    {
    std::cerr << "Caught unknown exception." << std::endl;
    return 1;
    }
