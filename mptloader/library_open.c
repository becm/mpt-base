/*!
 * load shared library, assign init/fini symbol
 */

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <dlfcn.h>

#ifndef MPT_NO_CONFIG
# include "config.h"
#else
# include <stdlib.h>
#endif

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief close handle
 * 
 * Delete library reference referred to by handle.
 * 
 * \param handle library handle
 * 
 * \return dynamic loader error message
 */
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *handle)
{
	const char *err = 0;
	
	if (handle->lib && dlclose(handle->lib) < 0 && (err = dlerror())) {
		errno = EINVAL;
	}
	handle->lib = 0;
	handle->create = 0;
	
	return err;
}

/*!
 * \ingroup mptLoader
 * \brief replace handle
 * 
 * Create reference to new dynamic library.
 * 
 * \param handle library handle
 * \param lib    library name to open
 * 
 * \return dynamic loader error message
 */
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *handle, const char *lib, const char *lpath)
{
	static const char dlopen_err[] = "unknown dlopen() failure";
	void *newlib;
	
	if (!lib) {
		static const char lib_err[] = "bad library path";
		return lib_err;
	}
	/* no resolution if path absolute */
	if (lpath && *lib != '/') {
		char buf[1024];
		size_t left, len;
		
		/* set library path */
		if ((len = strlen(lpath)) >= sizeof(buf)) {
			static const char prefix_err[] = "prefix exceeds buffer";
			errno = ERANGE;
			return prefix_err;
		}
		left = sizeof(buf) - len;
		buf[len] = '/';
		lpath = memcpy(buf, lpath, len++);
		newlib = buf + len;
		
		/* buffer too small */
		if (--left <= (len = strlen(lib))) {
			static const char err[] = "path exceeds buffer";
			errno = ERANGE;
			return err;
		}
		/* append library name to path */
		memcpy(newlib, lib, len+1);
		
		if (!(newlib = dlopen(buf, RTLD_NOW))) {
			return (newlib = dlerror()) ? newlib : dlopen_err;
		}
	}
	else if (!(newlib = dlopen(lib, RTLD_NOW))) {
		return (newlib = dlerror()) ? newlib : dlopen_err;
	}
	mpt_library_close(handle);
	handle->lib = newlib;
	
	return 0;
}

