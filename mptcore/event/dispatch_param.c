/*!
 * setup event controller for solver events
 */

#include <inttypes.h>

#include "meta.h"
#include "message.h"
#include "output.h"
#include "config.h"


#include "event.h"


static int getArg(MPT_STRUCT(message) *msg, int cmd, const char *_func)
{
	MPT_STRUCT(msgtype) hdr;
	
	/* consume command header */
	if (mpt_message_read(msg, sizeof(hdr), &hdr) < sizeof(hdr)) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("message header too small"));
		return MPT_ERROR(BadArgument);
	}
	/* consume command identifier */
	if (hdr.cmd != cmd) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("bad command type"));
		return MPT_ERROR(MissingData);
	}
	return hdr.arg;
}
/* config assignmet */
static int setConfig(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(config) *cfg = ptr;
	return cfg->_vptr->assign(cfg, p, val);
}
static int setEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char _func[] = "mpt_dispatch_param.set";
	MPT_INTERFACE(metatype) *conf = ptr;
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(message) tmp;
	int ret;
	
	if (!ev) {
		if (conf) {
			conf->_vptr->unref(conf);
		}
		return 0;
	}
	cfg = 0;
	if (conf) {
		if (MPT_metatype_convert(conf, MPT_type_pointer(MPT_ENUM(TypeConfig)), &cfg) < 0
		    || !cfg) {
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad config"));
		}
	}
	if (!ev->msg) {
		ret = mpt_config_set(cfg, 0, 0, 0, 0);
		if (ret < 0) {
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("no default config assign"));
		}
		return MPT_event_good(ev, MPT_tr("config default applied"));
	}
	/* consume command header */
	tmp = *ev->msg;
	if ((ret = getArg(&tmp, MPT_MESGTYPE(ParamSet), _func)) < 0) {
		return ret;
	}
	if (!cfg) {
		ret = mpt_message_assign(&tmp, ret, 0, 0);
	} else {
		ret = mpt_message_assign(&tmp, ret, setConfig, cfg);
	}
	if (ret < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("failed to assign config"));
	}
	return MPT_event_good(ev, MPT_tr("assigned config element"));
}
/* config query */
static int getEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char _func[] = "mpt_dispatch_param.get";
	MPT_INTERFACE(metatype) *conf = ptr;
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(message) tmp;
	
	int ret;
	
	if (!ev) {
		if (conf) {
			conf->_vptr->unref(conf);
		}
		return 0;
	}
	if (!ev->msg) {
		return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("no default config query"));
	}
	tmp = *ev->msg;
	if ((ret = getArg(&tmp, MPT_MESGTYPE(ParamGet), _func)) < 0) {
		return ret;
	}
	cfg = 0;
	if (conf) {
		if (MPT_metatype_convert(conf, MPT_type_pointer(MPT_ENUM(TypeConfig)), &cfg) < 0
		    || !cfg) {
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad config"));
		}
	}
	if (!ev->reply) {
		mpt_context_reply(0, 1, MPT_tr("no event reply context"));
		return MPT_EVENTFLAG(None);
	}
	ret = mpt_config_reply(ev->reply, cfg, ret, &tmp);
	if (ret < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("failed to get config elements"));
	}
	return MPT_EVENTFLAG(None);
}
/* conditional assignmet */
static int condConfig(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(config) *cfg = ptr;
	int ret;
	
	if (cfg->_vptr->query(cfg, p)) {
		return 0;
	}
	ret = cfg->_vptr->assign(cfg, p, val);
	return ret == 0 ? 1 : ret;
}
static int condEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char _func[] = "mpt_dispatch_param.cond";
	MPT_INTERFACE(metatype) *conf = ptr;
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(message) tmp;
	int ret;
	
	if (!ev) {
		if (conf) {
			conf->_vptr->unref(conf);
		}
		return 0;
	}
	if (!ev->msg) {
		return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("no default config assign"));
	}
	tmp = *ev->msg;
	if ((ret = getArg(&tmp, MPT_MESGTYPE(ParamCond), _func)) < 0) {
		return ret;
	}
	/* fallback to global config */
	if (!conf
	    && !(conf = mpt_config_global(0))) {
		return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("no global config"));
	}
	/* require valid config for replace */
	cfg = 0;
	if (MPT_metatype_convert(conf, MPT_type_pointer(MPT_ENUM(TypeConfig)), &cfg) < 0
	    || !cfg) {
		ret = MPT_ERROR(BadOperation);
	} else {
		ret = mpt_message_assign(&tmp, ret, condConfig, cfg);
	}
	/* unref global config */
	if (!ptr) {
		conf->_vptr->unref(conf);
	}
	if (ret < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("failed to set global config"));
	}
	if (!ret) {
		return MPT_event_good(ev, MPT_tr("keep config element"));
	}
	return MPT_event_good(ev, MPT_tr("assigned config element"));
}

static const char *errMesg(void)
{
	return MPT_tr("failed to set parameter handler");
}

/*!
 * \ingroup mptClient
 * \brief generic fallback commands
 * 
 * Set compatible event handlers in dispatch descriptor.
 * 
 * \param dsp  dispatch descriptor
 * \param mt   target metatype
 * 
 * \retval 0  success
 * \retval <0 failed to register reference
 */
extern int mpt_dispatch_param(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(metatype) *mt)
{
	uintptr_t id;
	
	/* global config dispatch */
	id = MPT_MESGTYPE(ParamSet);
	if (mpt_dispatch_set(dsp, id, setEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)",
		        errMesg(), id, "set");
		return MPT_ERROR(BadOperation);
	}
	if (mt && !mt->_vptr->addref(mt)) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %" PRIxPTR,
		        MPT_tr("unable to assign config query"), mt);
		return 1;
	}
	id = MPT_MESGTYPE(ParamGet);
	if (mpt_dispatch_set(dsp, id, getEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)",
		        errMesg(), id, "get");
		if (mt) {
			mt->_vptr->unref(mt);
		}
		return 1;
	}
	if (mt && !mt->_vptr->addref(mt)) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %" PRIxPTR,
		        MPT_tr("unable to assign conditiona config assign"), mt);
		return 2;
	}
	id = MPT_MESGTYPE(ParamCond);
	if (mpt_dispatch_set(dsp, id, condEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)",
		        errMesg(), id, "cond");
		if (mt) {
			mt->_vptr->unref(mt);
		}
		return 2;
	}
	return 3;
}
