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
	MPT_STRUCT(value) val;
	MPT_STRUCT(proxy) tmp;
	int len;
	
	if (!conf) {
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("missing initializer target"));
		return MPT_ERROR(BadArgument);
	}
	if ((len = mpt_proxy_type(&tmp, conf)) < 0) {
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Debug2), "%s: %s", MPT_tr("unspecified instance type"), conf);
	} else {
		conf += len;
	}
	/* create new proxy */
	if (!(m = mpt_meta_open(conf, path, out))) {
		if (!path || !(m = mpt_meta_open(conf, 0, out))) {
			return MPT_ERROR(BadOperation);
		}
	}
	val.ptr = 0;
	val.fmt = 0;
	
	/* set new proxy types */
	if (len > 0) {
		val.fmt = tmp._types;
		m->_vptr->assign(m, &val);
	}
	/* assign predefined proxy types */
	else if (len < 0 && *px->_types) {
		val.fmt = px->_types;
		m->_vptr->assign(m, &val);
		len = 0;
	}
	/* delete old proxy */
	if ((old = px->_ref)) {
		old->_vptr->unref(old);
	}
	px->_ref = m;
	
	memset(&px->_types, 0, sizeof(px->_types));
	*px->_types = MPT_ENUM(TypeMeta);
	
	if (out) {
		/* created object type */
		if (val.fmt && m->_vptr->conv(m, MPT_ENUM(TypeObject), &old) >= 0 && old) {
			const char *name;
			if (!(name = mpt_object_typename((void *) old))) {
				mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s", MPT_tr("created proxy object"));
			} else {
				mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("created proxy object"), name);
			}
		}
		/* generic instance */
		else {
			mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s", MPT_tr("created proxy instance"));
		}
	}
	
	return len;
}
