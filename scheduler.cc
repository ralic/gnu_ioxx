/*
 * $Source: /home/cvs/lib/libscheduler/scheduler.hpp,v $
 * $Revision: 1.18 $
 * $Date: 2001/01/26 18:59:47 $
 *
 * Copyright (c) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#include "scheduler.hpp"

// Destructors must exist, even if they're pure virtual.

scheduler::event_handler::~event_handler()
    {
    }
