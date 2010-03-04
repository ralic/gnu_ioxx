/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include "scheduler.hh"
using namespace std;

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
        cerr << "fd " << fd << " had a read timeout." << endl;
        }
    virtual void write_timeout(int fd)
        {
        cerr << "fd " << fd << " had a write timeout." << endl;
        }
    virtual void error_condition(int fd)
        {
        cerr << "fd " << fd << " had an error condition." << endl;
        }
    virtual void pollhup(int fd)
        {
        cerr << "fd " << fd << " has hung up." << endl;
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
    my_handler.prop.poll_events   = POLLIN;
    my_handler.prop.read_timeout  = 5;
    my_handler.prop.write_timeout = 3;
    sched.register_handler(0, my_handler, my_handler.prop);
    while (!sched.empty())
        {
        sched.schedule();
        //sched.dump(cerr);
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