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
	const char *cmd;
	int state;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	src = 0;
	if (ev->msg && !(src = mpt_event_command(ev))) {
		ev->id = 0;
		return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
	}
	/* consume command part */
	if (src) src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cmd);
	/* try to execute next solver step */
	state = cl->_vptr->step(cl, src);
	if (src) src->_vptr->ref.unref((void *) src);
	
	if (state < 0) {
		return MPT_event_fail(ev, state, MPT_tr("step operation failed"));
	}
	/* remaining solver steps */
	else if (state) {
		mpt_event_reply(ev, state, MPT_tr("step operation successfull"));
		return MPT_ENUM(EventNone);
	}
	mpt_event_reply(ev, state, MPT_tr("client run finished"));
	
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

