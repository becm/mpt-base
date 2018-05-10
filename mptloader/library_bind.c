/*!
 * bind solver according to config
 */

#include <dlfcn.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "loader.h"

static void msg(MPT_INTERFACE(logger) *info, const char *fcn, int type, const char *fmt, ... )
{
	if (!info && type >= MPT_LOG(Warning)) {
		return;
	}
	va_list va;
	va_start(va, fmt);
	if (info) {
		info->_vptr->log(info, fcn, type, fmt, va);
	} else {
		vfprintf(stderr, fmt, va);
	}
	va_end(va);
}

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
extern int mpt_library_bind(MPT_STRUCT(libsymbol) *lh, const char *conf, const char *path, MPT_INTERFACE(logger) *out)
{
	MPT_STRUCT(libsymbol) next = MPT_LIBSYMBOL_INIT;
	const char *libname;
	int ret = 0;
	
	if (!conf) {
		if (out) {
			msg(out, __func__, MPT_LOG(Error), "%s", MPT_tr("missing initializer target"));
		}
		return MPT_ERROR(BadArgument);
	}
	if ((libname = strchr(conf, '@'))) {
		ret = 1;
		++libname;
		/* load from special or default location */
		if (mpt_library_open(&next.lib, libname, path) < 0) {
			const char *err;
			if ((err = dlerror())) {
				ret = path ? MPT_LOG(Error) : MPT_LOG(Warning);
				msg(out, __func__, ret, "%s", err);
			}
			/* fallback to default library locations */
			if (!path || mpt_library_open(&next.lib, libname, 0) < 0) {
				if ((err = dlerror())) {
					msg(out, __func__, MPT_LOG(Error), "%s", err);
				}
				return MPT_ERROR(BadValue);
			}
			ret = 2;
		}
	}
	if (mpt_library_symbol(&next, conf) < 0) {
		const char *err;
		if (out && (err = dlerror())) {
			msg(out, __func__, MPT_LOG(Error), "%s", err);
		}
		if ((err = mpt_library_detach(&next.lib))) {
			if (!out) {
				abort();
			}
			msg(out, __func__, MPT_LOG(Fatal), "%s", err);
		}
		return MPT_ERROR(BadValue);
	}
	mpt_library_detach(&lh->lib);
	
	*lh = next;
	
	return ret;
}
