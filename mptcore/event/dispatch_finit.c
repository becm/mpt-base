/*!
 * init/fini MPT dispatch descriptor.
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/uio.h>

#include "array.h"
#include "output.h"
#include "message.h"
#include "event.h"

static int unknownEvent(void *arg, MPT_STRUCT(event) *ev)
{
	static const char _func[] = "mpt::event::unknown";
	MPT_INTERFACE(output) *out = arg;
	
	if (!ev) {
		if (out) {
			out->_vptr->obj.ref.unref((void *) out);
		}
		return 0;
	}
	if (ev->reply.set) {
		mpt_event_reply(ev, -1, "%s: 0x"PRIxPTR, MPT_tr("invalid message command"), ev->id);
		return MPT_ENUM(EventFail);
	}
	if (!out || ev->id) {
		mpt_output_log(out, _func, MPT_LOG(Error), "%s: 0x"PRIxPTR,
		               MPT_tr("invalid command"), ev->id);
		ev->id = 0;
		return MPT_ENUM(EventDefault);
	}
	if (!ev->msg) {
		mpt_output_log(out, _func, MPT_LOG(Info), "%s",
		               MPT_tr("empty message"));
		return 0;
	}
	return mpt_output_print(out, ev->msg);
}

/*!
 * \ingroup mptEvent
 * \brief initialize dispatch
 * 
 * Set initial values and reserve system resources.
 * 
 * \param disp  dispatch descriptor
 */
extern void mpt_dispatch_init(MPT_STRUCT(dispatch) *disp)
{
	disp->_d._buf = 0;
	
	disp->_def = 0;
	
	disp->_err.cmd = unknownEvent;
	disp->_err.arg = 0;
	
	disp->_ctx = 0;
}

/*!
 * \ingroup mptEvent
 * \brief close dispatch
 * 
 * Clear bound references and system resources.
 * 
 * \param disp  dispatch descriptor
 */
extern void mpt_dispatch_fini(MPT_STRUCT(dispatch) *disp)
{
	MPT_STRUCT(reply_context) *ctx;
	
	/* dereference registered commands */
	disp->_def  = 0;
	mpt_command_clear(&disp->_d);
	mpt_array_clone(&disp->_d, 0);
	
	if (disp->_err.cmd) {
		disp->_err.cmd(disp->_err.arg, 0);
		disp->_err.cmd = 0;
		disp->_err.arg = 0;
	}
	if ((ctx = disp->_ctx)) {
		MPT_INTERFACE(output) *out;
		if ((out = ctx->ptr)) {
			out->_vptr->obj.ref.unref((void *) out);
		}
		if (!ctx->used) {
			free(ctx);
		} else {
			ctx->ptr = 0;
		}
		disp->_ctx = 0;
	}
}
