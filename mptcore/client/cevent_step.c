/*!
 * execute solver step
 */

#include "event.h"
#include "message.h"
#include "meta.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client step
 * 
 * Execute generic client step operation.
 * Display step or final output (if applicable).
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (stop/continue/error)
 */
extern int mpt_cevent_step(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(iterator) *src;
	int state;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		ssize_t part;
		
		/* command message has content */
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) >= (ssize_t) sizeof(mt)
		    && mt.cmd == MPT_ENUM(MessageCommand)
		    && (part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			/* consume command part  */
			mpt_message_read(&msg, part+1, 0);
			/* create source for further arguments */
			if ((part = mpt_message_argv(&msg, mt.arg)) > 0
			    && !(src = mpt_meta_message(&msg, mt.arg))) {
				return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
			}
		}
	}
	/* try to execute next solver step */
	state = cl->_vptr->step(cl, src);
	if (src) src->_vptr->meta.ref.unref((void *) src);
	
	if (state < 0) {
		return MPT_event_fail(ev, state, MPT_tr("step operation failed"));
	}
	/* remaining solver steps */
	else if (state) {
		mpt_context_reply(ev->reply, state, MPT_tr("step operation successfull"));
		return MPT_EVENTFLAG(None);
	}
	return MPT_event_stop(ev, MPT_tr("client run finished"));
}

