/*
 * mpt C++ library
 *   event operations
 */

#include <poll.h>

#include "notify.h"

__MPT_NAMESPACE_BEGIN

template <> int type_properties<input *>::id(bool) {
	static const named_traits *traits = 0;
	if (traits || (traits = mpt_input_type_traits())) {
		return traits->type;
	}
	return BadType;
}

template<> inline const struct type_traits *type_properties<input *>::traits() {
	static const named_traits *traits = 0;
	if (traits || (traits = mpt_input_type_traits())) {
		return &traits->traits;
	}
	return 0;
}

template<> inline const struct type_traits *type_properties<reference<input> >::traits() {
	return mpt_input_reference_traits();
}

// input operations
int input::next(int)
{
	return -1;
}
int input::dispatch(event_handler_t cmd, void *arg)
{
	if (!cmd) {
		return -1;
	}
	event ev;
	return cmd(arg, &ev);
}

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

static int dispatch_event(void *arg, MPT_STRUCT(event) *ev)
{
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		ev = 0;
	}
	return mpt_dispatch_emit(static_cast<dispatch *>(arg), ev);
}
void notify::set_handler(dispatch *d)
{
	if (_disp.cmd) {
		_disp.cmd(_disp.arg, 0);
	}
	if ((_disp.arg = d)) {
		_disp.cmd = dispatch_event;
	} else {
		_disp.cmd = 0;
	}
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
