/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.cc,v $
 * $Revision: 1.1 $
 * $Date: 2001/03/19 14:34:44 $
 *
 * Copyright (c) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#include "scheduler.hh"

// Destructors must exist, even if they're pure virtual.

scheduler::event_handler::~event_handler()
    {
    }
