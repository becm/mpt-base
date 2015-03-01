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
 * \param solv  client descriptor
 * \param ev    event data
 * 
 * \return hint to event controller (stop/continue/error)
 */
extern int mpt_cevent_step(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
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
			mpt_output_log(solv->out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("bad message format"));
			return -1;
		}
	}
	if (!solv) {
		MPT_ABORT("missing client descriptor");
	}
	log = MPT_LOGGER((MPT_INTERFACE(metatype) *) solv->out);
	
	/* try to execute next solver step */
	while (step--) {
		if ((state = solv->_vptr->step(solv)) < 0) {
			char	buf[256];
			
			snprintf(buf, sizeof(buf), "%s: %d", MPT_tr("solver step failed"), state);
			buf[sizeof(buf)-1] = '\0';
			
			/* error output */
			solv->_vptr->output(solv, MPT_ENUM(OutputStateStep) | MPT_ENUM(OutputStateFail));
			solv->_vptr->report(solv, log);
			
			return MPT_event_fail(ev, buf);
		}
	}
	/* remaining solver steps */
	if (state) {
		const char *msg = MPT_tr("solver step successfull");
		solv->_vptr->output(solv, MPT_ENUM(OutputStateStep));
		if (ev->reply.set) {
			return MPT_event_good(ev, msg);
		} else {
			mpt_output_log(solv->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", msg);
			return MPT_ENUM(EventNone);
		}
	}
	mpt_event_reply(ev, 1, MPT_tr("solver run finished"));
	
	/* final iteration output */
	solv->_vptr->output(solv, MPT_ENUM(OutputStateFini));
	
	/* final solver report */
	solv->_vptr->report(solv, log);
	
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

