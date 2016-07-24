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

static int printReply(void *ptr, const MPT_STRUCT(message) *msg)
{
	static const char _func[] = "mpt::dispatch::reply";
	
	MPT_STRUCT(reply_context) *ctx;
	MPT_STRUCT(output) *ans;
	
	if (!(ctx = ptr)) {
		return 0;
	}
	if (!ctx->used) {
		mpt_log(0, _func, MPT_LOG(Critical), "%s",
		        MPT_tr("called with unregistered context"));
		return MPT_ERROR(MissingData);
	}
	if (!(ans = ctx->ptr)) {
		mpt_log(0, _func, MPT_LOG(Critical), "%s",
		        MPT_tr("context for answer destroyed"));
		if (!--ctx->used) {
			free(ctx);
		}
		return MPT_ERROR(BadArgument);
	}
	--ctx->used;
	if (!ctx->len) {
		mpt_output_log(ans, _func, MPT_LOG(Critical), "%s",
		               MPT_tr("reply committed"));
		return MPT_ERROR(BadArgument);
	}
	if (!msg) {
		uint64_t id = 0;
		mpt_message_buf2id(ctx->_val, ctx->len, &id);
		mpt_output_log(ans, _func, MPT_LOG(Debug), "%s (%"PRIx64")",
		               MPT_tr("empty reply"), id);
		ctx->len = 0;
		return 0;
	}
	ctx->len = 0;
	return mpt_output_print(ans, msg);
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
	MPT_INTERFACE(output) *out;
	MPT_STRUCT(reply_context) *ctx;
	const MPT_STRUCT(command) *cmd;
	int state;
	
	out = 0;
	if ((ctx = disp->_ctx)) {
		out = ctx->ptr;
	}
	/* execute default command */
	if (!ev) {
		/* check for default command */
		if (!(tmp.id = disp->_def)) {
			return 0;
		}
		/* bad default command */
		if (!(cmd = mpt_command_get(&disp->_d, tmp.id))) {
			disp->_def = 0;
			mpt_output_log(out, __func__, MPT_LOG(Critical), "%s (%"PRIxPTR")",
			               MPT_tr("invalid default command id"), tmp.id);
			return -2;
		}
		tmp.msg = 0;
		tmp.reply.set = 0;
		ev = &tmp;
	}
	/* explicit search of event id */
	else if (!ev->msg) {
		cmd = mpt_command_get(&disp->_d, ev->id);
	}
	/* find registered message handler */
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		uint8_t id;
		
		if (mpt_message_read(&msg, 1, &id) < 1) {
			return -2;
		}
		cmd = mpt_command_get(&disp->_d, ev->id = id);
	}
	/* fallback on dispatcher output */
	if (out && !ev->reply.set && !ctx->used) {
		ctx->len = sizeof(ev->id);
		++ctx->used;
		mpt_message_id2buf(ev->id, ctx->_val, ctx->len);
		
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
		mpt_event_reply(ev, MPT_ERROR(BadType), "%s: %"PRIxPTR, MPT_tr("unknown command"), ev->id);
		return -2;
	}
	/* bad execution of command */
	if (state < 0) {
		mpt_event_reply(ev, state, "%s: %"PRIxPTR,
		                MPT_tr("command execution failed"), ev->id);
		return state;
	}
	/* modify default command */
	if (state & MPT_ENUM(EventDefault)) {
		state &= ~MPT_ENUM(EventDefault);
		if (out) {
			if (ev->id) {
				if (disp->_def == ev->id) {
					mpt_output_log(out, __func__, MPT_LOG(Info), "%s (%"PRIxPTR")",
					               MPT_tr("keep default event"), disp->_def);
				} else if (disp->_def) {
					mpt_output_log(out, __func__, MPT_LOG(Info), "%s (%"PRIxPTR" > %"PRIxPTR")",
					               MPT_tr("default event replaced"), disp->_def, ev->id);
				} else {
					mpt_output_log(out, __func__, MPT_LOG(Info), "%s (%"PRIxPTR")",
					               MPT_tr("default event added"), ev->id);
				}
			}
			else if (disp->_def) {
				mpt_output_log(out, __func__, MPT_LOG(Info), "%s (%"PRIxPTR")",
				               MPT_tr("default event removed"), disp->_def);
			}
			else if (!(state & MPT_ENUM(EventFail))) {
				mpt_output_log(out, __func__, MPT_LOG(Debug2), "%s", MPT_tr("no default event to clear"));
			}
		}
		disp->_def = ev->id;
	}
	/* propagate default call availability */
	if (disp->_def) {
		state |= MPT_ENUM(EventDefault);
	}
	return state;
}

