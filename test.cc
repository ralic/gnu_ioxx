/*
 * $Source: /home/cvs/lib/libfastcgi/test.cpp,v $
 * $Revision: 1.7 $
 * $Date: 2000/08/17 13:58:28 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "scheduler.hpp"

class read_callback : public Scheduler::io_callback_t
    {
    string& buffer;
  public:
    bool eof;
    read_callback(string& buf) : buffer(buf), eof(false)
	{
	}
    bool operator() (int fd)
	{
	char    buf[1024];
	ssize_t rc = read(fd, buf, sizeof(buf));
	if (rc < 0)
	    throw runtime_error("read() failed");
	else if (rc == 0)
	    eof = true;
	else
	    buffer.append(buf, rc);
	return true;
	}
    };

class write_callback : public Scheduler::io_callback_t
    {
    string& buffer;
  public:
    bool empty;
    write_callback(string& buf) : buffer(buf), empty(buffer.empty())
	{
	}
    bool operator() (int fd)
	{
	ssize_t rc = write(fd, buffer.data(), buffer.size());
	if (rc < 0)
	    throw runtime_error("write() failed");
	else
	    buffer.erase(0, rc);
	empty = buffer.empty();
	return true;
	}
    };

int main()
try
    {
    string io_buffer;

    Scheduler sched;
    read_callback rc(io_buffer);
    write_callback wc(io_buffer);
    sched.register_event_handler(0, &rc);
    sched.register_event_handler(1, 0, 0, 0, &wc);

    while(!rc.eof && !wc.empty)
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
