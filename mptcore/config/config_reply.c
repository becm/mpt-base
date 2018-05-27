/*!
 * send event to return/error channel.
 */

#include <string.h>

#include <sys/uio.h>

#include "meta.h"
#include "event.h"
#include "message.h"
#include "output.h"

#include "config.h"

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
	const MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(message) tmp;
	int ret;
	
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
	while ((mt = mpt_config_message_next(cfg, sep, &tmp))) {
		const char *txt;
		size_t len;
		
		txt = mpt_meta_data(mt, 0);
		
		if (!rc) {
			int type = mt->_vptr->conv(mt, 0, 0);
			if (txt) {
				mpt_log(0, __func__, MPT_LOG(Debug), "%s (%d): %s",
				        MPT_tr("config value"), type, txt);
			} else {
				mpt_log(0, __func__, MPT_LOG(Debug), "%s: %d",
				        MPT_tr("config type"), type);
			}
			++ret;
			continue;
		}
		len = strlen(txt);
		if (!ret) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_MESGTYPE(ParamGet);
			hdr.arg = 0;
			if (!mpt_array_append(&arr, sizeof(hdr), &hdr)) {
				mpt_log(0, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("failed to add header"));
				break;
			}
		}
		else if (!mpt_array_append(&arr, 1, "")) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to add value separator"));
			break;
		}
		if (txt
		    && (len = strlen(txt))
		    && !mpt_array_append(&arr, len, txt)) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to add value"));
			break;
		}
		++ret;
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
