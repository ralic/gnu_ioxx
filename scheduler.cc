/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "scheduler.hh"

// Destructors must exist, even if they're pure virtual.

scheduler::event_handler::~event_handler()
    {
    }
