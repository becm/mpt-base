/*!
 * resolve and dispatch command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <sys/uio.h>

#include "array.h"
#include "message.h"
#include "output.h"

#include "event.h"

static int printReply(void *ptr, const MPT_STRUCT(message) *mptr)
{
	MPT_STRUCT(reply_context) *ctx;
	MPT_STRUCT(dispatch) *ans;
	MPT_STRUCT(event) ev;
	
	/* allow single message only */
	if (!(ctx = ptr)) {
		return MPT_ERROR(BadArgument);
	}
	/* context is not registered */
	if (!ctx->used) {
		mpt_log(0, "mpt::dispatch::reply", MPT_FCNLOG(Critical), "%s",
		        MPT_tr("called with unregistered context"));
		return MPT_ERROR(MissingData);
	}
	--ctx->used;
	/* connected dispatcher was deleted */
	if (!(ans = ctx->ptr)) {
		mpt_log(0, "mpt::dispatch::reply", MPT_FCNLOG(Critical), "%s",
		        MPT_tr("context for answer destroyed"));
		free(ctx);
		return MPT_ERROR(BadArgument);
	}
	/* no output target/source */
	if (!ans->_err.cmd) {
		return 0;
	}
	ev.id = 0;
	ev.msg = mptr;
	ev.reply.set = 0;
	ev.reply.context = 0;
	
	return ans->_err.cmd(ans->_err.arg, &ev);
}

/*!
 * \ingroup mptEvent
 * \brief dispatch event
 * 
 * Process event in dispatcher context.
 * 
 * \param disp dispatch controller
 * \param ev   event to process
 * 
 * \return result of dispatch operation
 */
extern int mpt_dispatch_emit(MPT_STRUCT(dispatch) *disp, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(event) tmp;
	MPT_STRUCT(reply_context) *ctx;
	const MPT_STRUCT(command) *cmd;
	int state;
	
	/* execute default command */
	if (!ev) {
		/* check for default command */
		if (!(tmp.id = disp->_def)) {
			return 0;
		}
		/* bad default command */
		if (!(cmd = mpt_command_get(&disp->_cmd, tmp.id))) {
			disp->_def = 0;
			mpt_log(0, __func__, MPT_FCNLOG(Critical), "%s (%"PRIxPTR")",
			        MPT_tr("invalid default command id"), tmp.id);
			return -2;
		}
		tmp.msg = 0;
		tmp.reply.set = 0;
		ev = &tmp;
	}
	/* explicit search of event id */
	else if (!ev->msg) {
		cmd = mpt_command_get(&disp->_cmd, ev->id);
	}
	/* find registered message handler */
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		uint8_t id;
		
		if (mpt_message_read(&msg, 1, &id) < 1) {
			return -2;
		}
		cmd = mpt_command_get(&disp->_cmd, ev->id = id);
	}
	/* fallback on dispatcher output */
	if (!ev->reply.set && (ctx = disp->_ctx)) {
		ctx->ptr = disp;
		ctx->len = sizeof(ev->id);
		++ctx->used;
		memcpy(ctx->_val, &ev->id, sizeof(ev->id));
		
		ev->reply.set = (int (*)()) printReply;
		ev->reply.context = ctx;
	}
	/* execute resolved command */
	if (cmd) {
		state = cmd->cmd(cmd->arg, ev);
	}
	/* default handler for unknown ids */
	else if (disp->_err.cmd) {
		state = disp->_err.cmd(disp->_err.arg, ev);
	}
	else {
		mpt_event_reply(ev, MPT_ERROR(BadType), "%s: "PRIxPTR, MPT_tr("unknown command"), ev->id);
		return -2;
	}
	/* bad execution of command */
	if (state < 0) {
		mpt_event_reply(ev, MPT_ERROR(BadArgument), "%s: "PRIxPTR,
		                MPT_tr("command execution failed"), ev->id);
		return state;
	}
	/* modify default command */
	if (state & MPT_ENUM(EventDefault)) {
		state &= ~MPT_ENUM(EventDefault);
		disp->_def = ev->id;
	}
	/* propagate default call availability */
	if (disp->_def) {
		state |= MPT_ENUM(EventDefault);
	}
	return state;
}

