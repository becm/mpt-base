/*!
 * init/fini MPT dispatch descriptor.
 */

#include <inttypes.h>

#include "meta.h"
#include "output.h"
#include "message.h"

#include "event.h"

static int unknownEvent(void *arg, MPT_STRUCT(event) *ev)
{
	static const char _func[] = "mpt::event::unknown";
	MPT_INTERFACE(logger) *log = arg;
	
	if (!ev) {
		return 0;
	}
	/* bad event id */
	if (ev->id) {
		if (ev->reply) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), "%s: %" PRIxPTR,
			                  MPT_tr("invalid command"), ev->id);
		} else {
			mpt_log(log, _func, MPT_LOG(Error), "%s: %" PRIxPTR,
			        MPT_tr("invalid command"), ev->id);
		}
		ev->id = 0;
		return MPT_EVENTFLAG(Default) | MPT_EVENTFLAG(Fail);
	}
	/* bad default event */
	if (!ev->msg) {
		if (ev->reply) {
			mpt_context_reply(ev->reply, MPT_ERROR(MissingData), "%s",
			                  MPT_tr("message required"));
		} else {
			mpt_log(log, _func, MPT_LOG(Error), "%s",
			        MPT_tr("message required"));
		}
		return MPT_EVENTFLAG(Default) | MPT_EVENTFLAG(Fail);
	}
	/* bad message type */
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		uint8_t type;
		
		if (!mpt_message_read(&msg, 1, &type)) {
			mpt_log(log, __func__, MPT_LOG(Warning), "%s", MPT_tr("empty message"));
			return MPT_EVENTFLAG(None);
		}
		if (ev->reply) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadOperation), "%s: %02x",
			                  MPT_tr("unable to process message type"), type);
		} else {
			mpt_log(log, _func, MPT_LOG(Error), "%s: %02x",
			        MPT_tr("unable to process message type"), type);
		}
		return MPT_EVENTFLAG(Fail);
	}
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
	MPT_STRUCT(metatype) *ctx;
	
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
		ctx->_vptr->unref(ctx);
		disp->_ctx = 0;
	}
}
