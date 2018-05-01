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

#include "loader.h"

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
extern void *mpt_library_open(const char *lib, const char *lpath)
{
	void *newlib;
	
	if (!lib) {
		dlerror();
		errno = EINVAL;
		return 0;
	}
	/* no resolution if path absolute */
	if (!lpath || *lib == '/') {
		return dlopen(lib, RTLD_NOW);
	}
	else {
		char buf[1024];
		size_t left, len;
		
		/* set library path */
		if ((len = strlen(lpath)) >= sizeof(buf)) {
			dlerror();
			errno = ERANGE;
			return 0;
		}
		left = sizeof(buf) - len;
		buf[len] = '/';
		memcpy(buf, lpath, len++);
		newlib = buf + len;
		
		/* buffer too small */
		if (--left <= (len = strlen(lib))) {
			dlerror();
			errno = ERANGE;
			return 0;
		}
		/* append library name to path */
		memcpy(newlib, lib, len + 1);
		
		return dlopen(buf, RTLD_NOW);
	}
}

