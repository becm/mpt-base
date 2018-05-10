/*!
 * load shared library, assign init/fini symbol
 */

#include <string.h>
#include <limits.h>

#include <dlfcn.h>
#include <stdlib.h>

#include "loader.h"

/* local private copy for refcount */
#define mpt_refcount_lower  __mpt_library_proxy_unref
#define mpt_refcount_raise  __mpt_library_proxy_addref
#include "misc/refcount.c"

/*!
 * \ingroup mptLoader
 * \brief close handle
 * 
 * Detach library reference referred to by handle.
 * 
 * \param handle library handle reference
 * 
 * \return dynamic loader error message
 */
extern const char *mpt_library_detach(MPT_STRUCT(libhandle) **handle)
{
	MPT_STRUCT(libhandle) *lh;
	uintptr_t count;
	
	if (!(lh = *handle)) {
		return 0;
	}
	if ((count = lh->_ref._val)
	    && __mpt_library_proxy_unref(&lh->_ref)) {
		return 0;
	}
	if (lh->addr && dlclose(lh->addr) < 0) {
		const char *err;
		if ((err = dlerror())) {
			return err;
		}
	}
	if (count) {
		free(lh);
	}
	*handle = 0;
	return 0;
}

/*!
 * \ingroup mptLoader
 * \brief replace handle
 * 
 * Create reference to new dynamic library.
 * 
 * \param lib    library name to open
 * \param lpath  library path
 * 
 * \return library handle
 */
extern int mpt_library_open(MPT_STRUCT(libhandle) **handle, const char *lib, const char *lpath)
{
	MPT_STRUCT(libhandle) *lh;
	void *newlib;
	
	if (!lib) {
		return MPT_ERROR(BadArgument);
	}
	/* no resolution if path absolute */
	if (!lpath || *lib == '/') {
		newlib = dlopen(lib, RTLD_NOW);
	}
	else {
		char buf[1024];
		size_t left, len;
		
		/* set library path */
		if ((len = strlen(lpath)) >= sizeof(buf)) {
			return MPT_ERROR(MissingBuffer);
		}
		left = sizeof(buf) - len;
		buf[len] = '/';
		memcpy(buf, lpath, len++);
		newlib = buf + len;
		
		/* buffer too small */
		if (--left <= (len = strlen(lib))) {
			return MPT_ERROR(MissingBuffer);
		}
		memcpy(newlib, lib, len + 1);
		newlib = dlopen(buf, RTLD_NOW);
	}
	if (!newlib) {
		return MPT_ERROR(BadValue);
	}
	if (!(lh = malloc(sizeof(*lh)))) {
		dlclose(newlib);
		dlerror();
		return MPT_ERROR(BadOperation);
	}
	lh->_ref._val = 1;
	lh->addr = newlib;
	
	mpt_library_detach(handle);
	*handle = lh;
	
	return 0;
}

