/*!
 * bind solver according to config
 */

#include <dlfcn.h>
#include <ctype.h>
#include <string.h>

#include "meta.h"
#include "object.h"

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
extern MPT_INTERFACE(metatype) *mpt_library_bind(uint8_t def, const char *conf, const char *path, MPT_INTERFACE(logger) *out)
{
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(libhandle) lh = MPT_LIBHANDLE_INIT;
	const char *err;
	int type;
	
	if (!conf) {
		if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s", MPT_tr("missing initializer target"));
		return 0;
	}
	if ((type = mpt_proxy_typeid(conf, &conf)) < 0) {
		if (!def && out) {
			mpt_log(out, __func__, MPT_LOG(Debug2), "%s: %s", MPT_tr("unknown instance type"), conf);
		}
		type = def;
	}
	if ((err = mpt_library_assign(&lh, conf, path))) {
		if (!path || (err = mpt_library_open(&lh, conf, 0))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s", err);
		}
		return 0;
	}
	/* create new proxy */
	if (!(m = mpt_library_meta(&lh, type))) {
		mpt_library_close(&lh);
		return 0;
	}
	/* created object type */
	if (m->_vptr->conv(m, MPT_ENUM(TypeObject), &obj) >= 0 && obj) {
		const char *name;
		if (!(name = mpt_object_typename(obj))) {
			mpt_log(out, __func__, MPT_LOG(Debug), "%s", MPT_tr("created proxy object"));
		} else {
			mpt_log(out, __func__, MPT_LOG(Debug), "%s: %s", MPT_tr("created proxy object"), name);
		}
	}
	/* generic instance */
	else {
		mpt_log(out, __func__, MPT_LOG(Debug), "%s: %02x", MPT_tr("created proxy instance"), type);
	}
	return m;
}
