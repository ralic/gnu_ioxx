/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.6 $
 * $Date: 2000/08/31 13:55:36 $
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
    int         out_fd;
    bool        eof;

  public:
    Pipe(Scheduler& sched_arg, int in_fd_arg, int out_fd_arg)
	    : sched(sched_arg), out_fd(out_fd_arg), eof(false)
	{
	sched.set_handler(in_fd_arg, this, POLLIN);
	}
    virtual ~Pipe()
	{
	}
    virtual void readable(int fd)
	{
	char buf[128];
	ssize_t len = read(fd, buf, sizeof(buf));
	if (len < 0)
	    throw runtime_error("read() failed.");
	else if (len == 0)
	    {
	    sched.set_handler(fd, 0, 0);
	    if (buffer.empty())
		{
		sched.set_handler(out_fd, 0, 0);
		delete this;
		}
	    else
		eof = true;
	    }
	else
	    {
	    buffer.append(buf, len);
	    sched.set_handler(out_fd, this, POLLOUT);
	    }
	cerr << "read() returned " << len << endl;
	}
    virtual void writable(int fd)
	{
	if (buffer.empty())
	    {
	    sched.set_handler(fd, 0, 0);
	    if (eof)
		delete this;
	    }
	else
	    {
	    ssize_t len = write(fd, buffer.data(), buffer.size());
	    if (len < 0)
		throw runtime_error("write() failed.");
	    else
		buffer.erase(0, len);
	    cerr << "write() returned " << len << endl;
	    }
	}
    };

int main()
try
    {
    Scheduler  sched;
    new Pipe(sched, 0, 1);
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
