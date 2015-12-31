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
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	int state, step = 1;
	
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)
		    || mt.cmd < MPT_ENUM(MessageCommand)
		    || mt.cmd >= MPT_ENUM(MessageUserMin)) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("bad message format"));
			return -1;
		}
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	log = mpt_output_logger(cl->out);
	
	/* try to execute next solver step */
	while (step--) {
		if ((state = cl->_vptr->step(cl)) < 0) {
			char buf[256];
			
			snprintf(buf, sizeof(buf), "%s: %d", MPT_tr("step operation failed"), state);
			buf[sizeof(buf)-1] = '\0';
			
			/* error output */
			cl->_vptr->output(cl, MPT_ENUM(OutputStateStep) | MPT_ENUM(OutputStateFail));
			cl->_vptr->report(cl, log);
			
			return MPT_event_fail(ev, buf);
		}
	}
	/* remaining solver steps */
	if (state) {
		const char *msg = MPT_tr("step operation successfull");
		cl->_vptr->output(cl, MPT_ENUM(OutputStateStep));
		if (ev->reply.set) {
			return MPT_event_good(ev, msg);
		} else {
			mpt_output_log(cl->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", msg);
			return MPT_ENUM(EventNone);
		}
	}
	mpt_event_reply(ev, 1, MPT_tr("client run finished"));
	
	/* final iteration output */
	cl->_vptr->output(cl, MPT_ENUM(OutputStateFini));
	
	/* final solver report */
	cl->_vptr->report(cl, log);
	
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

