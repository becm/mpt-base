/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "array.h"

#include "message.h"
#include "event.h"

#include "meta.h"
#include "output.h"

#include "client.h"

static int clientRead(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	int err;
	
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		if ((err = mpt_config_args((void *) cl, 0)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
		MPT_INTERFACE(metatype) *args = 0;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message type"));
		}
		if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part+1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part > 0 && !(args = mpt_meta_message(&msg, mt.arg))) {
			return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("unable to create argument source"));
		}
		err = mpt_config_args((void *) cl, args);
		args->_vptr->unref(args);
		
		if (err < 0) {
			if (args) {
				return MPT_event_fail(ev, err, MPT_tr("bad client config element"));
			} else {
				return MPT_event_fail(ev, err, MPT_tr(defFmt));
			}
		}
		if (err) {
			char buf[128];
			snprintf(buf, sizeof(buf), "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_event_good(ev, buf);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments applied to client configuration"));
}

static int clientClose(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	if (!ev) {
		cl->_vptr->cfg.unref((void *) cl);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
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
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message type"));
		}
		if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part, 0);
			if (mt.arg) mpt_message_read(&msg, 1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part >= 0) {
			if (part <= (ssize_t) msg.used) {
				if (!mt.arg && part && !((char *) msg.base)[part-1]) {
					--part;
				}
				id = mpt_hash(msg.base, part);
			}
			else if (part > 128) {
				return MPT_event_fail(ev, MPT_ERROR(MissingBuffer), MPT_tr("continue argument too large for buffer"));
			}
			else {
				char buf[128];
				mpt_message_read(&msg, part, buf);
				
				if (!mt.arg && part && !buf[part-1]) {
					--part;
				}
				id = mpt_hash(buf, part);
			}
		}
		else {
			id = mpt_hash("step", 4);
		}
	}
	/* configure default event to solver step */
	else {
		id = mpt_hash("step", 4);
	}
	
	if (!mpt_command_get(&d->_cmd, id)) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%s (%"PRIxPTR")", MPT_tr("invalid default command id"), id);
		return MPT_event_fail(ev, MPT_ERROR(BadValue), buf);
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
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
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
		return MPT_ERROR(BadArgument);
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
