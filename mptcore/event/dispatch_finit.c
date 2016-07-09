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
	MPT_INTERFACE(output) *out = arg;
	MPT_STRUCT(message) msg;
	char buf[128];
	size_t len;
	
	if (!ev) {
		if (out) {
			out->_vptr->obj.unref((void *) out);
		}
		return 0;
	}
	if (ev->reply.set) {
		mpt_event_reply(ev, -1, "%s: 0x"PRIxPTR, MPT_tr("invalid message command"), ev->id);
		return MPT_ENUM(EventFail);
	}
	if (!out || ev->id) {
		mpt_output_log(out, "mpt::event::unknown", MPT_FCNLOG(Error), "%s: 0x"PRIxPTR,
		               MPT_tr("invalid command"), ev->id);
		ev->id = 0;
		return MPT_ENUM(EventDefault);
	}
	if (!ev->msg) {
		mpt_output_log(out, "mpt::event::unknown", MPT_FCNLOG(Info), "%s",
		               MPT_tr("empty message"));
		return 0;
	}
	msg = *ev->msg;
	len = mpt_message_read(&msg, 2, buf);
	
	if (len < 1) {
		mpt_output_log(out, "mpt::event::unknown", MPT_FCNLOG(Info), "%s",
		               MPT_tr("empty message"));
		return 0;
	}
	if (buf[0] != MPT_ENUM(MessageOutput)
	    && buf[0] != MPT_ENUM(MessageAnswer)) {
		const char *fmt;
		/* print data info */
		switch (len + mpt_message_read(&msg, 1, buf+2)) {
		  case 1:  fmt = "{ %02x }"; break;
		  case 2:  fmt = "{ %02x, %02x }"; break;
		  case 3:  fmt = "{ %02x, %02x, %02x }"; break;
		  default: fmt = "{ %02x, %02x, ... }"; break;
		}
		len = snprintf(buf+2, sizeof(buf)-2, fmt, buf[0], buf[1], buf[2]);
		buf[0] = MPT_ENUM(MessageAnswer);
		buf[1] = 0;
		out->_vptr->push(out, len + 2, buf);
		out->_vptr->push(out, 0, 0);
		return 0;
	}
	out->_vptr->push(out, len, buf);
	while (1) {
		if (msg.used) {
			out->_vptr->push(out, msg.used, msg.base);
		}
		if (!msg.clen--) {
			break;
		}
		msg.base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		
		++msg.cont;
	}
	out->_vptr->push(out, 0, 0);
	return 0;
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
	disp->_cmd._buf = 0;
	
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
	mpt_command_clear(&disp->_cmd);
	mpt_array_clone(&disp->_cmd, 0);
	
	if (disp->_err.cmd) {
		disp->_err.cmd(disp->_err.arg, 0);
		disp->_err.cmd = 0;
		disp->_err.arg = 0;
	}
	if ((ctx = disp->_ctx)) {
		if (!ctx->used) {
			free(ctx);
		} else {
			ctx->ptr = 0;
		}
		disp->_ctx = 0;
	}
}
