/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.3 $
 * $Date: 2000/08/22 17:45:14 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <unistd.h>
#include "scheduler.hpp"

class read_callback : public Scheduler::io_callback_t
    {
    string& buffer;
  public:
    read_callback(string& buf) : buffer(buf) { }
    virtual ~read_callback() { }
    virtual void operator() (int fd)
	{
	char buf[10];
	ssize_t rc = read(fd, buf, sizeof(buf));
	cout << "read_callback called, read() returned " << rc << "." << endl;
	if (rc < 0)
	    throw runtime_error("read() failed.");
	else if (rc == 0)
	    {
	    // eof
	    }
	else
	    buffer.append(buf, rc);
	}
    };

class write_callback : public Scheduler::io_callback_t
    {
    string& buffer;
  public:
    write_callback(string& buf) : buffer(buf) { }
    virtual ~write_callback() { }
    virtual void operator() (int fd)
	{
	ssize_t rc = write(fd, buffer.data(), buffer.size());
	cout << "write_callback called, write() returned " << rc << "." << endl;
	if (rc < 0)
	    throw runtime_error("write() failed.");
	else
	    buffer.erase(0, rc);
	}
    };

int main()
try
    {
    string buf;
    read_callback  rc(buf);
    write_callback wc(buf);
    Scheduler      sched;

    sched.set_read_handler(0, &rc);
    sched.set_write_handler(2, &wc);
    sched.dump();
    for(;;)
	{
	sched.schedule();
	}

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
