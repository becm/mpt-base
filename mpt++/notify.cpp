/*
 * mpt C++ library
 *   event operations
 */

#include <limits>

#include <poll.h>

#include "array.h"

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

    ev.id  = 0;
    ev.msg = 0;
    ev.reply.set = 0;
    ev.reply.context = 0;

    return cmd(arg, &ev);
}
int input::_file()
{ return -1; }

// notifier operations
notify::notify()
{ mpt_notify_init(this); }
notify::~notify()
{ mpt_notify_fini(this); }
bool notify::add(input *in)
{ return mpt_notify_add(this, POLLIN, in) >= 0; }

input *notify::next() const
{ return mpt_notify_next(this); }
int notify::wait(int what, int wait)
{ return mpt_notify_wait(this, what, wait); }


// dispatcher
dispatch::dispatch()
{ mpt_dispatch_init(this); }
dispatch::~dispatch()
{ mpt_dispatch_fini(this); }
bool dispatch::set(uintptr_t id, EventHandler cmd, void *arg)
{ return mpt_dispatch_set(this, id, cmd, arg) >= 0; }
bool dispatch::setDefault(uintptr_t id)
{
    if (!mpt_command_get(&_cmd, id)) return false;
    _def = id;
    return true;
}
void dispatch::setError(int (*cmd)(void *, event *), void *arg)
{
    if (_err.cmd) {
        _err.cmd(_err.arg, 0);
    }
    _err.cmd = cmd;
    _err.arg = arg;
}


// message source interface
int MessageSource::reply(const message *)
{
    return -1;
}

__MPT_NAMESPACE_END
