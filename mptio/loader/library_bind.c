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
extern int mpt_library_bind(MPT_STRUCT(proxy) *px, const char *conf, MPT_INTERFACE(logger) *out)
{
	char buf[128];
	MPT_INTERFACE(metatype) *m;
	const char *ldesc;
	uintptr_t id, len;
	
	if (!conf) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogError) | MPT_ENUM(LogFunction), "%s", MPT_tr("missing initializer target"));
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
		(void) mpt_log(out, __func__, MPT_ENUM(LogError) | MPT_ENUM(LogFunction), "%s: %s", MPT_tr("invalid initializer length"), conf);
		return 0;
	}
	else {
		conf = memcpy(buf, conf, len);
		buf[len] = 0;
	}
	id = mpt_hash(conf, len);
	
	/* keep existing proxy object */
	if ((m = px->_mt) && (id == px->_id)) {
		(void) mpt_log(out, __func__, MPT_ENUM(LogDebug) | MPT_ENUM(LogFunction), "%s: %s", MPT_tr("instance types match"), conf);
		return 0;
	}
	/* create new proxy */
	if (!(m = mpt_meta_open(conf, out))) {
		return -1;
	}
	/* delete old proxy */
	if (px->_mt) {
		px->_mt->_vptr->unref(px->_mt);
	}
	px->_mt = m;
	px->_id = id;
	
	return len;
}
