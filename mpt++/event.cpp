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

/*!
 * \ingroup mptEvent
 * \brief get reply context interface traits
 * 
 * Get named traits for reply context pointer data.
 * 
 * \return named traits for reply context pointer
 */
const struct named_traits *reply_context::pointer_traits()
{
	return mpt_interface_traits(TypeReplyPtr);
}

// event reply andling
reply_context_detached *reply_context::defer()
{
	return 0;
}
bool reply_data::set(size_t len, const void *data)
{
	if (len && active()) {
		return false;
	}
	return (mpt_reply_set(this, len, data) < 0) ? false : true;
}

// command array
bool command::array::set_handler(uintptr_t id, event_handler_t cmd, void *arg)
{
	dispatch *d = static_cast<dispatch *>(this);
	return mpt_dispatch_set(d, id, cmd, arg) >= 0;
}
command *command::array::handler(uintptr_t id) const
{
	return mpt_command_get(this, id);
}
command *command::array::reserve(size_t len)
{
	return mpt_command_reserve(this, len);
}
// dispatcher
dispatch::dispatch()
{
	mpt_dispatch_init(this);
}
dispatch::~dispatch()
{
	mpt_dispatch_fini(this);
}
bool dispatch::set_default(uintptr_t id)
{
	if (!get(id)) {
		return false;
	}
	_def = id;
	return true;
}
void dispatch::set_error(event_handler_t cmd, void *arg)
{
	if (_err.cmd) {
		_err.cmd(_err.arg, 0);
	}
	_err.cmd = cmd;
	_err.arg = arg;
}

__MPT_NAMESPACE_END
