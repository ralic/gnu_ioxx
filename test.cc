/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.4 $
 * $Date: 2000/08/22 18:59:31 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <unistd.h>
#include "scheduler.hpp"

class Pipe : public Scheduler::EventHandler
    {
    string      buffer;
    Scheduler&  sched;

  public:
    Pipe(Scheduler& sched_arg) : sched(sched_arg)
	{
	}
    virtual ~Pipe()
	{
	}

    virtual void readable(int fd)
	{
	char buf[4*1024];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0)
	    throw runtime_error("read() failed.");
	else if (len == 0)
	    sched.set_handler(fd, 0, 0);
	else
	    {
	    buffer.append(buf, len);
	    sched.set_handler(1, this, POLLOUT);
	    }
	cerr << "read() returned " << len << endl;
	}

    virtual void writable(int fd)
	{
	if (buffer.empty())
	    {
	    sched.set_handler(fd, 0, 0);
	    return;
	    }

	ssize_t len = write(fd, buffer.data(), buffer.size());
	if (len < 0)
	    throw runtime_error("write() failed.");
	else
	    buffer.erase(0, len);
	cerr << "write() returned " << len << endl;
	}
    };

int main()
try
    {
    Scheduler  sched;
    Pipe*      pipe = new Pipe(sched);

    sched.set_handler(0, pipe, POLLIN);
    sched.set_handler(1, pipe, POLLOUT);
    sched.dump();
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
