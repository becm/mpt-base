/*!
 * bind solver according to config
 */

#include <dlfcn.h>
#include <ctype.h>
#include <string.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief proxy binding
 * 
 * Bind instance from shared library or local program.
 * Replace old binding atomicaly.
 * 
 * \param px   pointer to metatype proxy data
 * \param conf initializer function description
 * \param out  logging descriptor
 */
extern int mpt_library_bind(MPT_STRUCT(proxy) *px, const char *conf, const char *path, MPT_INTERFACE(logger) *out)
{
	MPT_INTERFACE(metatype) *m, *old;
	MPT_STRUCT(proxy) tmp;
	uintptr_t id;
	int len;
	
	if (!conf) {
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("missing initializer target"));
		return MPT_ERROR(BadArgument);
	}
	id = mpt_hash(conf, strlen(conf));
	
	/* keep existing proxy object */
	if ((old = px->_ref) && (id == px->_id)) {
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("instance types match"), conf);
		return 0;
	}
	memset(tmp._types, 0, sizeof(tmp._types));
	if ((len = mpt_proxy_type(&tmp, conf)) < 0) {
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Debug2), "%s: %s", MPT_tr("unspecified instance type"), conf);
		len = 0;
	}
	/* create new proxy */
	if (!(m = mpt_meta_open(tmp._types, conf+len, path, out))) {
		if (!path || !(m = mpt_meta_open(tmp._types, conf, 0, out))) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* delete old proxy */
	if (old) {
		old->_vptr->unref(old);
	}
	px->_ref = m;
	px->_id  = id;
	
	memset(&px->_types, 0, sizeof(px->_types));
	*px->_types = MPT_ENUM(TypeMeta);
	
	return len;
}
