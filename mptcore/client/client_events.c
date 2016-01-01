/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "client.h"

static int clientRead(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_INTERFACE(logger) *log;
	const char *err;
	
	if (!ev) return 0;
	
	if (!cl->conf && !(cl->conf = mpt_client_config("client"))) {
		return MPT_event_fail(ev, MPT_tr("unable to query configuration"));
	}
	log = mpt_output_logger(cl->out);
	
	if (!ev->msg) {
		err = mpt_client_read(cl->conf, 0, 0, log);
	}
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		size_t part;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
		/* consume command part */
		if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part, 0);
		}
		err = mpt_client_read(cl->conf, &msg, mt.arg, log);
	}
	if (err) {
		return MPT_event_fail(ev, err);
	}
	return MPT_event_good(ev, MPT_tr("reading configuration files completed"));
}

static int clientClose(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	if (!ev) {
		cl->_vptr->unref(cl);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
}

static int clientClear(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	if (!ev) return 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
	}
	cl->_vptr->clear(cl);
	return MPT_event_stop(ev, MPT_tr("solver cleared"));
}

static int clientGrapic(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_INTERFACE(output) *out;
	if (!ev) return 0;
	if (ev->msg && (out = cl->out)) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
		return mpt_output_graphic(out, ev);
	}
	return MPT_event_good(ev, MPT_tr("skip graphic binding"));
}

static const struct
{
	const char *name;
	int (*ctl)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
}
cmdsolv[] = {
	{"read",    clientRead      },
	{"init",    mpt_cevent_init },
	{"prep",    mpt_cevent_prep },
	{"step",    mpt_cevent_step },
	{"clear",   clientClear     },
	{"graphic", clientGrapic    },
	{"close",   clientClose     }
};

static int clientCont(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(dispatch) *d = ptr;
	uintptr_t id;
	
	if (!ev) return 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
	}
	/* configure default event to solver step */
	id = mpt_hash("step", 4);
	
	if (!mpt_command_get(&d->_cmd, id)) {
		mpt_output_log(d->_out, __func__, MPT_FCNLOG(Warning), "%s (%"PRIxPTR")",
		               MPT_tr("invalid default command id"), id);
		
		return MPT_event_fail(ev, MPT_tr("step operation not defined"));
	}
	mpt_output_log(d->_out, __func__, MPT_FCNLOG(Info), "%s (%"PRIxPTR")",
	               MPT_tr("assigned default command id"), id);
	
	ev->id = d->_def = id;
	
	return MPT_event_cont(ev, MPT_tr("step event registered"));
}

static int clientStop(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(dispatch) *d = ptr;
	
	if (!ev) return 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
	}
	if (d->_def) {
		mpt_output_log(d->_out, __func__, MPT_FCNLOG(Info), "%s (%"PRIxPTR")",
		               MPT_tr("cleared default command"), d->_def);
	}
	d->_def = 0;
	
	return MPT_event_stop(ev, MPT_tr("suspend solver run"));
}

/*!
 * \ingroup mptClient
 * \brief register client events
 * 
 * Set event handlers in dispatch descriptor.
 * 
 * \param dsp  dispatch descriptor
 * \param cl   client descriptor
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_client_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(client) *cl)
{
	uintptr_t id;
	size_t i;
	
	if (!dsp || !cl) {
		errno = EFAULT;
		return -1;
	}
	
	if (!cl->out && dsp->_out && dsp->_out->_vptr->obj.addref((void *) dsp->_out)) {
		cl->out = dsp->_out;
	}
	
	/* mapping of command type messages */
	id = MPT_ENUM(MessageCommand);
	if (mpt_dispatch_set(dsp, id, (int (*)()) mpt_dispatch_hash, dsp) < 0) {
		mpt_output_log(dsp->_out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to set string command handler"));
		return -1;
	}
	/* register default event activator */
	id = mpt_hash("cont", 4);
	if (mpt_dispatch_set(dsp, id, clientCont, dsp) < 0) {
		mpt_output_log(dsp->_out, __func__, MPT_FCNLOG(Error), "%s: %"PRIxPTR" (%s)\n",
		               MPT_tr("error registering handler id"), id, "cont");
	}
	/* register default event activator */
	id = mpt_hash("stop", 4);
	if (mpt_dispatch_set(dsp, id, clientStop, dsp) < 0) {
		mpt_output_log(dsp->_out, __func__, MPT_FCNLOG(Error), "%s: %"PRIxPTR" (%s)\n",
		               MPT_tr("error registering handler id"), id, "stop");
	}
	/* register client command handler */
	for (i = 0; i < MPT_arrsize(cmdsolv); i++) {
		id = mpt_hash(cmdsolv[i].name, strlen(cmdsolv[i].name));
		
		if (mpt_dispatch_set(dsp, id, (int (*)()) cmdsolv[i].ctl, cl) < 0) {
			mpt_output_log(dsp->_out, __func__, MPT_FCNLOG(Warning), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("error registering handler id"), id, cmdsolv[i].name);
		}
	}
	return 0;
}
