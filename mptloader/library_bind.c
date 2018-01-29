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
extern int mpt_library_bind(MPT_STRUCT(libhandle) *lh, const char *conf, const char *path, MPT_INTERFACE(logger) *out)
{
	const char *libname, *sname;
	void *(*sym)(void);
	void *lib;
	int ret = 0;
	
	if (!(sname = conf)) {
		if (out) {
			mpt_log(out, __func__, MPT_LOG(Error), "%s", MPT_tr("missing initializer target"));
		}
		return MPT_ERROR(BadArgument);
	}
	if (lh->type < 0) {
		if (out) {
			mpt_log(out, __func__, MPT_LOG(Debug2), "%s: %s", MPT_tr("unknown instance type"), conf);
		}
		return MPT_ERROR(BadType);
	}
	lib = 0;
	if ((libname = strchr(sname, '@'))) {
		ret = 1;
		/* load from special or default location */
		if (!(lib = mpt_library_open(libname + 1, path))) {
			sname = dlerror();
			if (sname && out) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s", sname);
			}
			/* fallback to default library locations */
			if (!path || !(lib = mpt_library_open(libname + 1, 0))) {
				sname = dlerror();
				mpt_log(out, __func__, MPT_LOG(Error), "%s", sname);
				return MPT_ERROR(BadValue);
			}
			ret = 2;
		}
	}
	if (!(sym = mpt_library_symbol(lib, sname))) {
		sname = dlerror();
		mpt_log(out, __func__, MPT_LOG(Error), "%s", sname);
		if (lib) {
			dlclose(lib);
		}
		return MPT_ERROR(BadValue);
	}
	mpt_library_close(lh);
	
	lh->lib = lib;
	lh->create = sym;
	lh->hash = mpt_hash(sname, strlen(sname));
	
	return ret;
}
