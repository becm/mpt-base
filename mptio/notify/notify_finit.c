/*!
 * init/fini notifier.
 */

#include <inttypes.h>
#include <unistd.h>

#if defined(__linux__)
# include <sys/epoll.h>
#endif

#include "array.h"

#include "event.h"
#include "message.h"

#include "output.h"

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
/*!
 * \ingroup mptNotify
 * \brief initialize notification descriptor
 * 
 * Set initial values and reserve system resources.
 * 
 * \param no  notification descriptor
 */
extern void mpt_notify_init(MPT_STRUCT(notify) *no)
{
	no->_disp.cmd = 0;
	no->_disp.arg = 0;
	
	no->_slot._buf = no->_wait._buf = 0;
	
	no->_sysfd = -1;
	no->_fdused = 0;
}

/*!
 * \ingroup mptNotify
 * \brief close notification descriptor
 * 
 * Clear bound references and system resources.
 * 
 * \param no  notification descriptor
 */
extern void mpt_notify_fini(MPT_STRUCT(notify) *no)
{
	/* clear event handler */
	if (no->_disp.cmd) {
		no->_disp.cmd(no->_disp.arg, 0);
		no->_disp.cmd = 0;
		no->_disp.arg = 0;
	}
	/* clear slot data */
	mpt_array_callunref(&no->_slot);
	mpt_array_clone(&no->_slot, 0);
	
	/* clear temporal data */
	mpt_array_clone(&no->_wait, 0);
	
	if (no->_sysfd >= 0) {
		(void) close(no->_sysfd);
	}
	no->_fdused = 0;
	no->_sysfd = -1;
}
