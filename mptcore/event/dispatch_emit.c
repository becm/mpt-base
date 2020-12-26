/*!
 * resolve and dispatch command.
 */

#include <inttypes.h>

#include "meta.h"
#include "message.h"
#include "output.h"
#include "types.h"

#include "event.h"

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
	MPT_STRUCT(event) tmp = MPT_EVENT_INIT;
	const MPT_STRUCT(command) *cmd;
	int state;
	
	/* execute default command */
	if (!ev) {
		/* check for default command */
		if (!(tmp.id = disp->_def)) {
			return 0;
		}
		/* bad default command */
		if (!(cmd = mpt_command_get(&disp->_d, tmp.id))) {
			disp->_def = 0;
			mpt_log(0, __func__, MPT_LOG(Critical), "%s (%" PRIxPTR ")",
			        MPT_tr("invalid default command id"), tmp.id);
			return MPT_ERROR(BadValue);
		}
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
	if (!ev->reply) {
		MPT_INTERFACE(metatype) *mt;
		if ((mt = disp->_ctx)) {
			MPT_metatype_convert(mt, MPT_ENUM(TypeReplyPtr), &ev->reply);
		}
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
		mpt_context_reply(ev->reply, MPT_ERROR(BadType), "%s: %" PRIxPTR, MPT_tr("unknown command"), ev->id);
		return MPT_ERROR(BadArgument);
	}
	/* bad execution of command */
	if (state < 0) {
		mpt_context_reply(ev->reply, state, "%s: %" PRIxPTR,
		                  MPT_tr("command execution failed"), ev->id);
		return state;
	}
	/* modify default command */
	if (state & MPT_EVENTFLAG(Default)) {
		state &= ~MPT_EVENTFLAG(Default);
		if (ev->id) {
			if (disp->_def == ev->id) {
				mpt_log(0, __func__, MPT_DISPATCH_LOG_ACTION, "%s (%" PRIxPTR ")",
				        MPT_tr("keep default event"), disp->_def);
			} else if (disp->_def) {
				mpt_log(0, __func__, MPT_DISPATCH_LOG_ACTION, "%s (%" PRIxPTR " > %" PRIxPTR ")",
				        MPT_tr("default event replaced"), disp->_def, ev->id);
			} else {
				mpt_log(0, __func__, MPT_DISPATCH_LOG_ACTION, "%s (%" PRIxPTR ")",
				        MPT_tr("default event added"), ev->id);
			}
		}
		else if (disp->_def) {
			mpt_log(0, __func__, MPT_DISPATCH_LOG_ACTION, "%s (%" PRIxPTR ")",
			        MPT_tr("default event removed"), disp->_def);
		}
		else if (!(state & MPT_EVENTFLAG(Fail))) {
			mpt_log(0, __func__, MPT_DISPATCH_LOG_STATUS, "%s", MPT_tr("no default event to clear"));
		}
		disp->_def = ev->id;
	}
	/* propagate default call availability */
	if (disp->_def) {
		state |= MPT_EVENTFLAG(Default);
	}
	return state;
}

