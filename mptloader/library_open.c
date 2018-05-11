/*!
 * load shared library, assign init/fini symbol
 */

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <dlfcn.h>
#include <stdlib.h>

#include "loader.h"

/* local private copy for refcount */
#define mpt_refcount_lower  __mpt_library_proxy_unref
#define mpt_refcount_raise  __mpt_library_proxy_addref
#include "misc/refcount.c"

/*!
 * \ingroup mptLoader
 * \brief attach handle
 * 
 * Add new reference to library handle.
 * 
 * \param handle library handle to reference
 * 
 * \return new reference for library handle
 */
extern MPT_STRUCT(libhandle) *mpt_library_attach(MPT_STRUCT(libhandle) *lh)
{
	if (!lh) {
		errno = EINVAL;
		return 0;
	}
	if (!__mpt_library_proxy_addref(&lh->_ref)) {
		errno = EAGAIN;
		return 0;
	}
	return lh;
}
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
extern int mpt_library_detach(MPT_STRUCT(libhandle) **handle)
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
		return MPT_ERROR(BadOperation);
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
extern MPT_STRUCT(libhandle) *mpt_library_open(const char *lib, const char *lpath)
{
	MPT_STRUCT(libhandle) *lh;
	void *newlib;
	
	if (!lib) {
		errno = EINVAL;
		return 0;
	}
	/* no resolution if path absolute */
	if (!lpath || *lib == '/') {
		if (!(newlib = dlopen(lib, RTLD_NOW))) {
			errno = ENOENT;
			return 0;
		}
	}
	else while (1) {
		const char *sep;
		char buf[1024];
		size_t left, len;
		
		if (!lpath || !*lpath) {
			errno = ENOENT;
			return 0;
		}
		if ((sep = strchr(lpath, ':'))) {
			len = sep++ - lpath;
		} else {
			len = strlen(lpath);
		}
		/* set library path */
		if (len >= sizeof(buf)) {
			errno = ENOBUFS;
			return 0;
		}
		left = sizeof(buf) - len;
		memcpy(buf, lpath, len);
		buf[len++] = '/';
		newlib = buf + len;
		
		/* buffer too small */
		if (--left <= (len = strlen(lib))) {
			errno = ENOBUFS;
			return 0;
		}
		memcpy(newlib, lib, len + 1);
		
		if ((newlib = dlopen(buf, RTLD_NOW))) {
			break;
		}
		lpath = sep;
	}
	if (!(lh = malloc(sizeof(*lh)))) {
		dlclose(newlib);
		dlerror();
		errno = ENOMEM;
		return 0;
	}
	lh->_ref._val = 1;
	lh->addr = newlib;
	
	return lh;
}

