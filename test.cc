/*
 * $Source: /home/cvs/lib/libscheduler/test.cpp,v $
 * $Revision: 1.1 $
 * $Date: 2000/08/21 15:48:30 $
 *
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>
#include "scheduler.hpp"

struct read_callback : public Scheduler::io_callback_t
    {
    virtual ~read_callback() { }
    virtual bool operator() (int) { return true; }
    };

int main()
try
    {
    read_callback rc;
    Scheduler     sched;

    sched.set_read_handler(7, &rc);
    sched.set_read_handler(2, &rc);
    sched.set_read_handler(3, &rc);
    sched.set_read_handler(3, &rc);
    sched.set_read_handler(1, &rc);
    sched.set_read_handler(0, &rc);
    sched.set_read_handler(9, &rc);
    sched.set_read_handler(0, 0);
    sched.set_read_handler(7, &rc);
    sched.set_read_handler(5, &rc);
    sched.set_read_handler(5, &rc);
    sched.set_read_handler(4, &rc);
    sched.dump();

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
