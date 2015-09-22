/*!
 * resolve and dispatch command.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <sys/uio.h>

#include "array.h"
#include "message.h"
#include "event.h"

static int printReply(void *ptr, const MPT_STRUCT(message) *mptr)
{
	MPT_STRUCT(message) msg;
	MPT_STRUCT(output) *ans;
	const uint8_t *base;
	
	/* allow single message only */
	if (!(ans = *((void **) ptr))) {
		return -3;
	}
	/* add reply data */
	if (!mptr) {
		return 0;
	}
	msg = *mptr;
	
	while (!msg.used) {
		if (!msg.clen--) {
			ans->_vptr->push(ans, 0, 0);
			return 0;
		}
		msg.base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		++msg.cont;
	}
	base = msg.base;
	
	/* catch non-print messages */
	if ((base[0] != MPT_ENUM(MessageOutput))
	    && (base[0] != MPT_ENUM(MessageAnswer))) {
		const char *fmt;
		char buf[256];
		ssize_t len;
		
		msg.used = mpt_message_read(&msg, 4, buf + (sizeof(buf)-4));
		
		/* print data info */
		switch (msg.used) {
		  case 1:  fmt = "{ %02x }"; break;
		  case 2:  fmt = "{ %02x, %02x }"; break;
		  case 3:  fmt = "{ %02x, %02x, %02x }"; break;
		  default: fmt = "{ %02x, %02x, ... }"; break;
		}
		len = snprintf(buf+2, sizeof(buf)-6, fmt,
		               buf[sizeof(buf)-4],
		               buf[sizeof(buf)-3],
		               buf[sizeof(buf)-2],
		               buf[sizeof(buf)-1]);
		buf[0] = MPT_ENUM(MessageAnswer);
		buf[1] = 0;
		
		/* output buffered/transformed message */
		if ((len = ans->_vptr->push(ans, len + 2, buf)) < 0) {
			return len;
		}
		return 1;
	}
	
	while (1) {
		ssize_t curr = ans->_vptr->push(ans, msg.used, base);
		
		if (curr < 0 || (size_t) curr > msg.used) {
			if (ans->_vptr->push(ans, 1, 0) < 0) {
				return -1;
			} else {
				return -2;
			}
		}
		if ((msg.used -= curr)){
			base += curr;
			continue;
		}
		if (!msg.clen--) {
			if (ans->_vptr->push(ans, 0, 0) >= 0) {
				return 0;
			}
			if (ans->_vptr->push(ans, 1, 0) < 0) {
				return -1;
			} else {
				return -2;
			}
		}
		msg.base = base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		
		++msg.cont;
	}
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
	MPT_STRUCT(output) *ans;
	MPT_STRUCT(event) tmp;
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
			mpt_output_log(disp->_out, __func__, MPT_FCNLOG(Critical), "%s (%"PRIxPTR")",
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
	if (!ev->reply.set && (ans = disp->_out)) {
		ev->reply.set = (int (*)()) printReply;
		ev->reply.context = &ans;
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
		mpt_output_log(disp->_out, __func__, MPT_FCNLOG(Warning), "%s (%"PRIx8")",
		               MPT_tr("invalid command id"), ev->id);
		return -2;
	}
	/* bad execution of command */
	if (state < 0) {
		mpt_output_log(disp->_out, __func__, MPT_FCNLOG(Debug), "%s (%"PRIxPTR")",
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

