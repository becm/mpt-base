/*!
 * send event to return/error channel.
 */

#include <string.h>

#include <sys/uio.h>

#include "event.h"
#include "message.h"
#include "meta.h"
#include "output.h"
#include "types.h"

#include "config.h"

static int _add_reply(void *ctx, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *col)
{
	static const char *_func = "mpt_config_reply";
	MPT_STRUCT(array) *arr = ctx;
	const char *txt = mpt_convertable_data(val, 0);
	int len;
	
	(void) col;
	
	if (!arr) {
		int type = val->_vptr->convert(val, 0, 0);
		if (txt) {
			mpt_log(0, _func, MPT_LOG(Debug), "%s (%d): %s",
			        MPT_tr("config value"), type, txt);
		} else {
			mpt_log(0, _func, MPT_LOG(Debug), "%s: %d",
			        MPT_tr("config type"), type);
		}
		return 0;
	}
	len = strlen(txt);
	
	if (!arr->_buf || !arr->_buf->_used) {
		MPT_STRUCT(msgtype) hdr;
		hdr.cmd = MPT_MESGTYPE(ParamGet);
		hdr.arg = 0;
		if (!mpt_array_append(arr, sizeof(hdr), &hdr)) {
			mpt_log(0, _func, MPT_LOG(Error), "%s",
			        MPT_tr("failed to add header"));
			return MPT_ERROR(MissingBuffer);
		}
	}
	else if (!mpt_array_append(arr, 1, "")) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("failed to add value separator"));
		return MPT_ERROR(MissingBuffer);
	}
	if (txt
	    && (len = strlen(txt))
	    && !mpt_array_append(arr, len, txt)) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("failed to add value"));
		return MPT_ERROR(MissingBuffer);
	}
	return 0;
}

/*!
 * \ingroup mptConfig
 * \brief collect config values
 * 
 * Set reply message to values from path elements.
 * 
 * \param rc    target reply context
 * \param cfg   config source
 * \param sep   path separator
 * \param msg   path separator
 * 
 * \return number of arguments found
 */
extern int mpt_config_reply(MPT_INTERFACE(reply_context) *rc, const MPT_INTERFACE(config) *cfg, int sep, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	MPT_STRUCT(path) path = MPT_PATH_INIT;
	MPT_INTERFACE(metatype) *global = 0;
	MPT_STRUCT(message) tmp;
	int ret;
	
	if (msg && !cfg) {
		global = mpt_config_global(0);
		if (!global
		  || (MPT_metatype_convert(global, MPT_ENUM(TypeConfigPtr), &cfg) < 0)
		  || !cfg) {
			global->_vptr->unref(global);
			msg = 0;
		}
	}
	
	/* status reply */
	if (!msg) {
		int8_t val;
		if (!rc) {
			return 0;
		}
		val = MPT_MESGTYPE(ParamGet);
		tmp.base = &val;
		tmp.used = sizeof(val);
		tmp.cont = 0;
		tmp.clen = 0;
		return rc->_vptr->reply(rc, &tmp);
	}
	
	tmp = *msg;
	ret = 0;
	while (mpt_config_message_next(&path, sep, &tmp) > 0) {
		int r = cfg->_vptr->query(cfg, &path, _add_reply, rc ? &arr : 0);
		if (r < 0) {
			if (!ret) {
				ret = r;
			}
			break;
		}
		++ret;
	}
	if (global) {
		global->_vptr->unref(global);
	}
	if (!rc) {
		return ret;
	}
	if (!arr._buf) {
		mpt_context_reply(rc, ret, "%s", MPT_tr("failed to get config elements"));
		return 0;
	}
	tmp.base = arr._buf + 1;
	tmp.used = arr._buf->_used;
	tmp.cont = 0;
	tmp.clen = 0;
	ret = rc->_vptr->reply(rc, &tmp);
	
	mpt_array_clone(&arr, 0);
	
	return ret;
}
