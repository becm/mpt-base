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
	va_list va;
	va_start(va, fmt);
	info->_vptr->log(info, fcn, type, fmt, va);
	vfprintf(stderr, fmt, va);
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
		if (!(next.lib = mpt_library_open(libname, path))) {
			const char *err;
			if (out && (err = dlerror())) {
				ret = path ? MPT_LOG(Error) : MPT_LOG(Warning);
				msg(out, __func__, ret, "%s", err);
			}
			/* fallback to default library locations */
			if (!path || !(next.lib = mpt_library_open(libname, 0))) {
				if (out && (err = dlerror())) {
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
		if ((ret = mpt_library_detach(&next.lib)) < 0) {
			if (!out) {
				abort();
			}
			if ((err = dlerror())) {
				msg(out, __func__, MPT_LOG(Fatal), "%s", err);
			}
			return ret;
		}
		return MPT_ERROR(BadValue);
	}
	mpt_library_detach(&lh->lib);
	
	*lh = next;
	
	return ret;
}
