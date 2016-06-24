/*!
 * execute solver step
 */

#include <stdio.h>
#include <errno.h>

#include "event.h"
#include "message.h"
#include "meta.h"

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
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message type"));
		}
		/* consume command part  */
		if ((part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			mpt_message_read(&msg, part+1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part >= 0 && !(src = mpt_meta_message(&msg, mt.arg))) {
			return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
		}
	}
	/* try to execute next solver step */
	state = cl->_vptr->step(cl, src);
	if (src) src->_vptr->unref(src);
	
	if (state < 0) {
		return MPT_event_fail(ev, state, MPT_tr("step operation failed"));
	}
	/* remaining solver steps */
	else if (state) {
		mpt_event_reply(ev, state, MPT_tr("step operation successfull"));
		return MPT_ENUM(EventNone);
	}
	mpt_event_reply(ev, 1, MPT_tr("client run finished"));
	
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

