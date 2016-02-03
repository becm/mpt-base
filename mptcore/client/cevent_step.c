/*!
 * execute solver step
 */

#include <stdio.h>
#include <errno.h>

#include "event.h"
#include "message.h"
#include "array.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client solve step
 * 
 * Execute single client solve step.
 * Display step or final output (if applicable).
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (stop/continue/error)
 */
extern int mpt_cevent_step(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *src;
	int state;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)
		    || mt.cmd < MPT_ENUM(MessageCommand)
		    || mt.cmd >= MPT_ENUM(MessageUserMin)) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("bad message format"));
			return -1;
		}
		/* consume command part  */
		part = mpt_message_argv(&msg, mt.arg);
		mpt_message_read(&msg, part, 0);
		if (mt.arg) mpt_message_read(&msg, 1, 0);
		part = mpt_message_argv(&msg, mt.arg);
		
		if (part >= 0 && !(src = mpt_meta_message(&msg, mt.arg))) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("failed to create argument stream"));
			return -1;
		}
	}
	/* try to execute next solver step */
	state = cl->_vptr->step(cl, src);
	if (src) src->_vptr->unref(src);
	
	if (state < 0) {
		char buf[256];
		
		snprintf(buf, sizeof(buf), "%s: %d", MPT_tr("step operation failed"), state);
		buf[sizeof(buf)-1] = '\0';
		return MPT_event_fail(ev, buf);
	}
	/* remaining solver steps */
	else if (state) {
		const char *msg = MPT_tr("step operation successfull");
		if (ev->reply.set) {
			return MPT_event_good(ev, msg);
		} else {
			mpt_output_log(cl->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", msg);
			return MPT_ENUM(EventNone);
		}
	}
	mpt_event_reply(ev, 1, MPT_tr("client run finished"));
	
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

