/*
 * mpt C++ library
 *   event operations
 */

#include <limits>

#include <poll.h>

#include "array.h"
#include "output.h"

#include "event.h"
#include "message.h"
#include "notify.h"

__MPT_NAMESPACE_BEGIN

// input operations
int input::next(int)
{
    return -1;
}
int input::dispatch(EventHandler cmd, void *arg)
{
    if (!cmd) return -1;
    event ev;
    return cmd(arg, &ev);
}
int input::_file()
{ return -1; }

// notifier operations
notify::notify() : _sysfd(-1), _fdused(0)
{
    _disp.cmd = 0;
    _disp.arg = 0;
}
notify::~notify()
{
    mpt_notify_fini(this);
}

bool notify::add(input *in)
{
    return mpt_notify_add(this, POLLIN, in) >= 0;
}
bool notify::init(int argc, char * const argv[])
{
    return mpt_init(this, argc, argv) >= 0;
}

void notify::setDispatch(dispatch *d)
{
    mpt_notify_setdispatch(this, d);
}

input *notify::next() const
{
    return mpt_notify_next(this);
}
int notify::wait(int what, int wait)
{
    return mpt_notify_wait(this, what, wait);
}
bool notify::loop()
{
    return mpt_loop(this) >= 0;
}

__MPT_NAMESPACE_END
