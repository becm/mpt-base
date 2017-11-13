/*!
 * setup event controller for solver events
 */

#include <inttypes.h>
#include <string.h>

#include "message.h"
#include "config.h"
#include "event.h"

#include "meta.h"

#include "client.h"

static int setEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
	MPT_INTERFACE(logger) *log = 0;
	MPT_INTERFACE(config) *cfg = 0;
	int err;
	
	if (!ev) {
		return 0;
	}
	if ((err = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0) {
		return MPT_event_fail(ev, err, MPT_tr("no config element"));
	}
	mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log);
	
	if (!ev->msg) {
		if ((err = mpt_config_args(cfg, 0, log)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_INTERFACE(iterator) *it;
		
		if (!(mt = mpt_event_command(ev))
		    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it) < 0
		    || !it) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		/* process config arguments */
		err = mpt_config_args(cfg, it, log);
		mt->_vptr->ref.unref((void *) mt);
		
		if (err < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad config element"));
		}
		if (err) {
			mpt_context_reply(ev->reply, 0, "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_EVENTFLAG(None);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments applied to configuration"));
}
static int delEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
	MPT_INTERFACE(logger) *log = 0;
	MPT_INTERFACE(config) *cfg = 0;
	int err;
	
	if (!ev) {
		return 0;
	}
	if ((err = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0
	    || !cfg) {
		return MPT_event_fail(ev, err, MPT_tr("no config element"));
	}
	mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log);
	
	if (!ev->msg) {
		if ((err = mpt_config_clear(cfg, 0, log)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_INTERFACE(iterator) *it;
		
		if (!(mt = mpt_event_command(ev))
		    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it) < 0
		    || !it) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		/* process config arguments */
		err = mpt_config_clear(cfg, it, log);
		mt->_vptr->ref.unref((void *) mt);
		
		if (err < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad config element"));
		}
		if (err) {
			mpt_context_reply(ev->reply, 0, "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_EVENTFLAG(None);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments removed from configuration"));
}

static int closeEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_INTERFACE(reference) *ref = ptr;
	
	if (!ev) {
		ref->_vptr->unref(ref);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		part = mpt_message_read(&msg, sizeof(mt), &mt);
		if (part == (ssize_t) sizeof(mt)
		    && mt.cmd == MPT_ENUM(MessageCommand)
		    && (part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			mpt_message_read(&msg, part + 1, 0);
			if ((part = mpt_message_argv(&msg, mt.arg)) >= 0) {
				const char *msg = MPT_tr("close can not handle arguments");
				return MPT_event_fail(ev, MPT_ERROR(BadArgument), msg);
			}
		}
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
}

static int continueEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(dispatch) *d = ptr;
	const char *cmd = "step";
	uintptr_t id;
	
	if (!ev) {
		return 0;
	}
	id = mpt_hash(cmd, strlen(cmd));
	if (ev->msg) {
		MPT_INTERFACE(metatype) *src;
		MPT_INTERFACE(iterator) *it;
		const char *next;
		int ret;
		
		if (!(src = mpt_event_command(ev))
		    || src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it) < 0
		    || !it) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		/* use second command element */
		if ((ret = it->_vptr->get(it, 's', &next)) > 0
		    && (ret = it->_vptr->advance(it)) >= 0
		    && (ret = it->_vptr->get(it, 's', &next)) > 0
		    && next) {
			id = mpt_hash(next, strlen(next));
		}
		src->_vptr->ref.unref((void *) src);
	}
	/* require registered id as default */
	if (!mpt_command_get(&d->_d, id)) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%" PRIxPTR ")",
		                  MPT_tr("invalid default command id"), id);
		ev->id = 0;
		return MPT_EVENTFLAG(Default) | MPT_EVENTFLAG(Fail);
	}
	mpt_context_reply(ev->reply, 2, "%s: %s (%" PRIxPTR ")", MPT_tr("register default event"), cmd, id);
	
	ev->id = id;
	
	return MPT_EVENTFLAG(Default);
}
static int stopEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(dispatch) *d = ptr;
	
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		}
	}
	if (d->_def) {
		mpt_context_reply(ev->reply, 2, "%s (%" PRIxPTR ")", MPT_tr("clear default event"), d->_def);
	} else {
		mpt_context_reply(ev->reply, 0, "%s", MPT_tr("clear default event"));
	}
	ev->id = 0;
	
	return MPT_EVENTFLAG(Default);
}

static int hashEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	if (!ev) {
		return 0;
	}
	return mpt_dispatch_hash(ptr, ev);
}

static int clientProcess(void *ptr, void *ev)
{
	MPT_INTERFACE(client) *cl = ptr;
	if (!ev) {
		return 0;
	}
	return cl->_vptr->process(cl, ev);
}

static int clientEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(client) *cl = ptr;
	if (!ev) {
		cl->_vptr->meta.ref.unref((void *) cl);
		return 0;
	}
	return cl->_vptr->process(cl, ev);
}

/*!
 * \ingroup mptClient
 * \brief register client events
 * 
 * Set event handlers in dispatch descriptor.
 * 
 * \param dsp  dispatch descriptor
 * \param meta target metatype
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_meta_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(client) *cl;
	uintptr_t id;
	
	if (!dsp || !mt) {
		return MPT_ERROR(BadArgument);
	}
	/* enable client dispatch */
	id = MPT_ENUM(MessageCommand);
	cl = 0;
	if (mt->_vptr->conv(mt, mpt_client_typeid(), &cl) >= 0
	    && cl) {
		MPT_STRUCT(command) *ctl;
		/* mapping of command type messages */
		if ((ctl = mpt_command_get(&dsp->_d, id))) {
			if (ctl->cmd(ctl->arg, 0) < 0) {
				return MPT_ERROR(BadOperation);
			}
			ctl->cmd = clientProcess;
			ctl->arg = cl;
		}
		if (dsp->_err.cmd) {
			dsp->_err.cmd(dsp->_err.arg, 0);
		}
		dsp->_err.cmd = clientEvent;
		dsp->_err.arg = cl;
		return 0;
	}
	/* mapping of command type messages */
	if (mpt_dispatch_set(dsp, id, hashEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("unable to set string command handler"));
		return MPT_ERROR(BadOperation);
	}
	/* register default event activator */
	id = mpt_hash("cont", 4);
	if (mpt_dispatch_set(dsp, id, continueEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "cont");
	}
	id = mpt_hash("continue", 8);
	if (mpt_dispatch_set(dsp, id, continueEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "continue");
	}
	/* register default event activator */
	id = mpt_hash("stop", 4);
	if (mpt_dispatch_set(dsp, id, stopEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "stop");
	}
	/* register default event activator */
	id = mpt_hash("close", 5);
	if (mpt_dispatch_set(dsp, id, closeEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "close");
	}
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), 0) < 0) {
		return 0;
	}
	/* enable configuration dispatch */
	id = mpt_hash("set", 3);
	if (mpt_dispatch_set(dsp, id, setEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "stop");
	}
	id = mpt_hash("del", 3);
	if (mpt_dispatch_set(dsp, id, delEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "del");
	}
	return 0;
}
