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
			mpt_log(0, "mpt_dispatch_emit", MPT_LOG(Error), "%s: 0x"PRIxPTR,
			        MPT_tr("unprocessed reply id"), ev->id);
		}
		return disp->_def ? MPT_ENUM(EventDefault) : 0;
	}
	/* trigger default event */
	if (!ev->msg) {
		ev = 0;
	}
	return mpt_dispatch_emit(disp, ev);
}
/*!
 * \ingroup mptNotify
 * \brief set notify dispatcher
 * 
 * Assign dispatcher to notification descriptor.
 * Finalize dispatcher on controller change and notify finalize.
 * 
 * \param no   notification descriptor
 * \param disp event dispatch bindings
 */
extern void mpt_notify_setdispatch(MPT_STRUCT(notify) *no, MPT_STRUCT(dispatch) *disp)
{
	if (no->_disp.cmd) {
		no->_disp.cmd(no->_disp.arg, 0);
		no->_disp.cmd = 0;
	}
	if ((no->_disp.arg = disp)) {
		no->_disp.cmd = dispatchEvent;
	}
}
