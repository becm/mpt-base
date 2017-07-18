/*!
 * init/fini notifier.
 */

#include <stdlib.h>
#include <inttypes.h>

#include "notify.h"

static int dispatchEvent(void *arg, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(dispatch) *disp = arg;
	if (!ev) {
		mpt_dispatch_fini(disp);
		free(disp);
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
extern MPT_STRUCT(dispatch) *mpt_notify_dispatch(MPT_STRUCT(notify) *no)
{
	MPT_STRUCT(dispatch) *disp;
	
	if (!(disp = malloc(sizeof(*disp)))) {
		return 0;
	}
	if (no->_disp.cmd) {
		no->_disp.cmd(no->_disp.arg, 0);
		no->_disp.cmd = 0;
	}
	mpt_dispatch_init(disp);
	no->_disp.cmd = dispatchEvent;
	no->_disp.arg = disp;
	
	return disp;
}
