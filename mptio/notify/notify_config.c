/*!
 * register input with type system.
 */

#include <poll.h>

#include "types.h"
#include "config.h"
#include "output.h"

#include "notify.h"

static int _notify_connect(void *ctx, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *sub)
{
	static const char *_func = "mpt_notify_config.connect";
	MPT_INTERFACE(input) *in = 0;
	const char *ctl = 0;
	MPT_STRUCT(notify) *no = ctx;
	const MPT_STRUCT(named_traits) *traits;
	
	(void) sub;
	
	if ((traits = mpt_input_type_traits())
	  && (val->_vptr->convert(val, traits->type, &in) >= 0)) {
		int ret;
		if (!in || !in->_vptr->meta.addref((void *) in)) {
			mpt_log(0, _func, MPT_LOG(Error), "%s",
			        "no valid connection reference");
			return in ? MPT_ERROR(BadOperation) : MPT_ERROR(BadValue);
		}
		if ((ret = mpt_notify_add(no, POLLIN, in)) < 0) {
			return ret;
		}
		return 1;
	}
	
	if (val->_vptr->convert(val, 's', &ctl) >= 0) {
		int ret;
		if (!ctl) {
			return 0;
		}
		if ((ret = mpt_notify_connect(no, ctl)) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        "failed to connect to controller", ctl);
			return ret;
		}
		return 2;
	}
	
	return MPT_ERROR(BadType);
}

static int _notify_listen(void *ctx, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *sub)
{
	static const char *_func = "mpt_notify_config.listen";
	MPT_INTERFACE(input) *in = 0;
	const char *ctl = 0;
	MPT_STRUCT(notify) *no = ctx;
	const MPT_STRUCT(named_traits) *traits;
	
	(void) sub;

	if ((traits = mpt_input_type_traits())
	  && (val->_vptr->convert(val, traits->type, &in) >= 0)) {
		int ret;
		if (!in || !in->_vptr->meta.addref((void *) in)) {
			mpt_log(0, _func, MPT_LOG(Error), "%s",
			        "no valid input reference");
			return in ? MPT_ERROR(BadOperation) : MPT_ERROR(BadValue);
		}
		if ((ret = mpt_notify_add(no, POLLIN, in)) < 0) {
			return ret;
		}
		return 1;
	}
	
	if (val->_vptr->convert(val, 's', &ctl) >= 0) {
		int ret;
		if (!ctl) {
			return 0;
		}
		if ((ret = mpt_notify_bind(no, ctl, 2)) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        "failed to create controller", ctl);
		}
		return 2;
	}
	
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptNotify
 * \brief apply connection settings
 * 
 * Add input settings from config to notifier.
 * 
 * \return added input count
 */
extern int mpt_notify_config(MPT_STRUCT(notify) *no, const MPT_INTERFACE(config) *cfg)
{
	static const char con[] = "mpt.connect";
	static const char bind[] = "mpt.listen";
	MPT_INTERFACE(metatype) *global;
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	int off, cret, bret, ncon;
	
	if (!cfg) {
		if (!(global = mpt_config_global(0))
		  || MPT_metatype_convert(global, MPT_ENUM(TypeConfigPtr), &cfg) < 0
		  || !cfg) {
			return MPT_ERROR(MissingData);
		}
	}
	
	off = global ? 0 : 4;
	
	ncon = 0;
	mpt_path_set(&path, con + off, -1);
	if ((cret = cfg->_vptr->query(cfg, &path, _notify_connect, no)) > 0) {
		ncon++;
	}
	
	mpt_path_set(&path, bind + off, -1);
	if ((bret = cfg->_vptr->query(cfg, &path, _notify_listen, no)) > 0) {
		++ncon;
		
	}
	if (global) {
		global->_vptr->unref(global);
	}
	if (!ncon) {
		if (bret < 0 && bret != MPT_ERROR(MissingData)) {
			return bret;
		}
		if (cret < 0 && cret != MPT_ERROR(MissingData)) {
			return cret;
		}
	}
	return ncon;
}
