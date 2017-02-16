/*!
 * init/fini notifier.
 */

#include <inttypes.h>

#include "notify.h"

static int dispatchEvent(void *arg, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(dispatch) *disp = arg;
	if (!ev) {
		mpt_dispatch_fini(disp);
		return 0;
	}
	/* process reply notification */
	if (ev->id) {
		if (ev->msg) {
			mpt_log(0, "mpt_dispatch_emit", MPT_LOG(Error), "%s: 0x" PRIxPTR,
			        MPT_tr("unprocessed reply id"), ev->id);
		}
		return disp->_def ? MPT_EVENTFLAG(Default) : 0;
	}
	/* trigger default event */
	if (!ev->msg) {
		ev = 0;
	}
	return mpt_dispatch_emit(disp, ev);
}

static int ignoreEvent(void *arg, MPT_STRUCT(event) *ev)
{
	(void) arg;
	if (!ev || !ev->reply) {
		return 0;
	}
	mpt_context_reply(ev->reply, 0, "%s", MPT_tr("event ignored"));
	return MPT_EVENTFLAG(None);
}
/*!
 * \ingroup mptNotify
 * \brief set notify dispatcher
 * 
 * Assign dispatcher to notification descriptor.
 * Dispatcher is finalized on controller change.
 * 
 * \param no   notification descriptor
 * \param disp event dispatch bindings
 */
extern void mpt_notify_setdispatch(MPT_STRUCT(notify) *no, MPT_STRUCT(dispatch) *disp)
{
	size_t id;
	if (no->_disp.cmd) {
		no->_disp.cmd(no->_disp.arg, 0);
		no->_disp.cmd = 0;
	}
	
	if (!(no->_disp.arg = disp)) {
		no->_disp.cmd = ignoreEvent;
		return;
	}
	no->_disp.cmd = dispatchEvent;
	if (disp->_def || no->_fdused) {
		return;
	}
	/* use start event in dispatcher */
	id = mpt_hash("start", 5);
	if (mpt_command_get(&disp->_d, id)) {
		disp->_def = id;
	}
}
