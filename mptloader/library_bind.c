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
	char buf[128];
	MPT_INTERFACE(metatype) *m, *old;
	const char *ldesc;
	uintptr_t id, len;
	
	if (!conf) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("missing initializer target"));
		return -1;
	}
	/* resolve alias to full library description */
	ldesc = conf;
	while (*ldesc && !isspace(*ldesc)) {
		++ldesc;
	}
	len = ldesc - conf;
	if (!*ldesc) {
		;
	}
	else if ((len = ldesc - conf) >= sizeof(buf)) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("invalid initializer length"), conf);
		return 0;
	}
	else {
		conf = memcpy(buf, conf, len);
		buf[len] = 0;
	}
	id = mpt_hash(conf, len);
	
	/* keep existing proxy object */
	if ((old = px->_ref) && (id == px->_id)) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("instance types match"), conf);
		return 0;
	}
	/* create new proxy */
	if (!(m = mpt_meta_open(conf, path, out))) {
		if (path && !(m = mpt_meta_open(conf, 0, out))) {
			return -1;
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
