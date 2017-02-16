/*
 * mpt C++ library
 *   event operations
 */

#include <limits>

#include <stdlib.h>

#include <poll.h>

#include "array.h"
#include "output.h"

#include "event.h"

__MPT_NAMESPACE_BEGIN

// event reply andling
reply_context *reply_context::defer()
{
    return 0;
}
bool reply_data::setData(size_t len, const void *data)
{
    if (len && active()) return false;
    return (mpt_reply_set(this, len, data) < 0) ? false : true;
}

// command array
bool command::array::set(uintptr_t id, EventHandler cmd, void *arg)
{
    dispatch *d = static_cast<dispatch *>(this);
    return mpt_dispatch_set(d, id, cmd, arg) >= 0;
}
command *command::array::get(uintptr_t id)
{
    return mpt_command_get(this, id);
}
command *command::array::next(size_t len)
{
    return mpt_command_nextid(this, len);
}
// dispatcher
dispatch::dispatch()
{ mpt_dispatch_init(this); }
dispatch::~dispatch()
{ mpt_dispatch_fini(this); }
bool dispatch::setDefault(uintptr_t id)
{
    if (!get(id)) return false;
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
    return 0;
}

__MPT_NAMESPACE_END
