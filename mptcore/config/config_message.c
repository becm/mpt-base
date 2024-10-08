/*!
 * send event to return/error channel.
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"
#include "meta.h"
#include "message.h"
#include "output.h"
#include "types.h"

#include "config.h"

static MPT_INTERFACE(config) *getGlobal(MPT_INTERFACE(metatype) **glob, const char *_func)
{
	MPT_INTERFACE(config) *cfg;
	MPT_INTERFACE(metatype) *mt;
	
	if (!(mt = mpt_config_global(0))) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("failed to get global config"));
		return 0;
	}
	cfg = 0;
	if (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg) < 0
	    || !cfg) {
		mpt_log(0, _func, MPT_LOG(Error), "%s",
		        MPT_tr("no interface for global config"));
		mt->_vptr->unref(mt);
		return 0;
	}
	*glob = mt;
	return cfg;
}
static int setConfig(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(config) *cfg = ptr;
	return cfg->_vptr->assign(cfg, p, val);
}
/*!
 * \ingroup mptConfig
 * \brief set config elements
 * 
 * Assign element data described by message in config.
 * 
 * \param msg   message content
 * \param len   number of path elements
 * \param proc  process assignment
 * \param ctx   assignment context
 * 
 * \return result of assign operation
 */
extern int mpt_message_assign(const MPT_STRUCT(message) *msg, int len, int (*proc)(void *ptr, const MPT_STRUCT(path) *, const MPT_STRUCT(value) *), void *ctx)
{
	MPT_INTERFACE(metatype) *glob;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(message) tmp;
	MPT_STRUCT(value) val = MPT_VALUE_INIT(MPT_type_toVector('c'), 0);
	struct iovec vec;
	char buf[1024];
	size_t all;
	int ret;
	
	glob = 0;
	if (!proc) {
		if (!(ctx = getGlobal(&glob, __func__))) {
			return MPT_ERROR(BadOperation);
		}
		proc = setConfig;
	}
	if (!msg) {
		ret = proc(ctx, 0, 0);
		if (glob) {
			glob->_vptr->unref(glob);
		}
		return ret;
	}
	tmp = *msg;
	if (len < 0) {
		MPT_STRUCT(msgtype) hdr;
		
		if (mpt_message_read(&tmp, sizeof(hdr), &hdr) < sizeof(hdr)) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("message header too small"));
			return MPT_ERROR(MissingData);
		}
		len = hdr.arg;
	}
	all = mpt_message_read(&tmp, sizeof(buf), buf);
	if (all >= sizeof(buf)) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("no message arguments"));
		if (glob) {
			glob->_vptr->unref(glob);
		}
		return MPT_ERROR(MissingBuffer);
	}
	/* set target path */
	p.base = buf;
	p.len = 0;
	p.sep = 0;
	
	/* set path element length */
	while (len--) {
		const char *end, *start = p.base + p.off + p.len;
		if (!(end = memchr(start, 0, all - p.len))) {
			if (glob) {
				glob->_vptr->unref(glob);
			}
			return MPT_ERROR(BadValue);
		}
		++end;
		p.len += end - start;
		start = end;
	}
	
	vec.iov_base = (char *) p.base + p.off + p.len;
	vec.iov_len  = all - p.off - p.len;
	
	val._addr = &vec;
	
	ret = proc(ctx, &p, &val);
	
	if (glob) {
		glob->_vptr->unref(glob);
	}
	return ret;
}

/*!
 * \ingroup mptConfig
 * \brief process path elements
 * 
 * Process path elements in message data.
 * 
 * \param msg   message content
 * \param sep   path separator
 * \param proc  path handler
 * \param ctx   handler context
 * 
 * \return number of processed path elements
 */
extern int mpt_config_message_next(MPT_STRUCT(path) *path, int sep, MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(message) tmp;
	char *addr;
	int len;
	tmp = *msg;
	
	/* consume header argument */
	if (sep < 0) {
		int8_t arg;
		
		len = mpt_message_read(&tmp, sizeof(arg), &arg);
		
		if (!len) {
			mpt_path_set(path, 0, 0);
			return 0;
		}
		sep = arg;
	}
	else if ((len = mpt_message_argv(&tmp, sep)) < 0) {
		mpt_log(0, __func__, MPT_LOG(Info), "%s",
		        MPT_tr("no arguments path info in message"));
		return MPT_ERROR(MissingData);
	}
	if (!(buf = _mpt_buffer_alloc(len + 1, 0))) {
		int max = sizeof(buf) - 1;
		mpt_log(0, __func__, MPT_LOG(Error), "%s (%d < %d)",
		        MPT_tr("path buffer too small"), max, len);
		return MPT_ERROR(MissingBuffer);
	}
	addr = (char *) (buf + 1);
	addr[len] = 0;
	mpt_path_fini(path);
	path->base = addr;
	path->flags |= MPT_PATHFLAG(HasArray);
	
	/* consume path and separator */
	len = mpt_message_read(&tmp, len, addr);
	if (sep >= 0) {
		mpt_message_read(&tmp, 1, 0);
	}
	*msg = tmp;
	return len ? len : 1;
}
