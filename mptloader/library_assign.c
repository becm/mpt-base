/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <string.h>

#include <dlfcn.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief assign library handle
 * 
 * Connect file handle to symbol in library.
 * 
 * \param handle library handle
 * \param descr  symbol @ library to open
 * 
 * \return dynamic loader error message
 */
extern const char *mpt_library_assign(MPT_STRUCT(libhandle) *handle, const char *descr, const char *lpath)
{
	char buf[128], *libname;
	MPT_STRUCT(libhandle) lh = MPT_LIBHANDLE_INIT;
	const char *err;
	union {
		void *ptr;
		void *(*fcn)(void);
	} tmp;
	
	if (!descr) {
		return MPT_tr("no target defined");
	}
	if ((libname = strchr(descr, '@'))) {
		size_t len;
		if ((len = libname++ - descr) >= sizeof(buf)) {
			return MPT_tr("symbol name exceeds temporary buffer");
		}
		if (!len) {
			return MPT_tr("no init symbol defined");
		}
		descr = memcpy(buf, descr, len);
		buf[len] = '\0';
		if (!*libname || isspace(*libname)) {
			libname = 0;
		}
	}
	if (libname && (err = mpt_library_open(&lh, libname, lpath))) {
		return err;
	}
	/* bind controller to symbol */
	if (!(tmp.ptr = dlsym(lh.lib, descr))) {
		static const char dlsym_err[] = "unknown dlsym() failure";
		if (lh.lib) {
			dlclose(lh.lib);
		}
		return (err = dlerror()) ? err : dlsym_err;
	}
	mpt_library_close(handle);
	handle->lib = lh.lib;
	handle->create = tmp.fcn;
	
	return 0;
}
