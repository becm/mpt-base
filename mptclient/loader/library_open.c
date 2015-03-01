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

static char *get_prefix()
{
#ifdef MPT_NO_CONFIG
	return getenv("MPT_PREFIX");
#else
	MPT_INTERFACE(metatype) *conf = mpt_config_get(0, "mpt.prefix", '.', 0);
	return conf ? conf->_vptr->typecast(conf, 's') : 0;
#endif
}
static char *get_prefix_lib()
{
#ifdef MPT_NO_CONFIG
	return getenv("MPT_PREFIX_LIB");
#else
	MPT_INTERFACE(metatype) *conf = mpt_config_get(0, "mpt.prefix.lib", '.', 0);
	return conf ? conf->_vptr->typecast(conf, 's') : 0;
#endif
}

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
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *handle, const char *lib)
{
	static const char dlopen_err[] = "unknown dlopen() failure";
	static const char prefix_err[] = "prefix exceeds buffer";
	static const char path_err[] = "path exceeds buffer";
	
	void *newlib;
	
	if (!lib) {
		static const char lib_err[] = "bad library path";
		return lib_err;
	}
	/* search library in standard location(s) */
	if (!(newlib = dlopen(lib, RTLD_NOW))) {
		char	*lpath, buf[1024];
		size_t	left, len;
		
		/* set library path */
		if ((lpath = get_prefix_lib())) {
			if ((left = strlen(lpath)) >= sizeof(buf)) {
				errno = ERANGE; return prefix_err;
			}
			lpath = ((char *) memcpy(buf, lpath, left)) + left;
			left = sizeof(buf) - left;
		}
		/* create library path */
		else if ((lpath = get_prefix())) {
			if ( (left = strlen(lpath)) >= (sizeof(buf)-4) ) {
				errno = ERANGE; return prefix_err;
			}
			lpath = memcpy(buf, lpath, left);
			lpath = memcpy(lpath + left, "/lib", 4);
			lpath += 4;
			left = (sizeof(buf)-4) - left;
		}
		/* no path info, direct fail */
		else {
			newlib = dlerror();
			return newlib ? newlib : dlopen_err;
		}
		if ( --left <= (len = strlen(lib)) ) {
			errno = ERANGE; return path_err;
		}
		/* append library name to path */
		*(lpath++) = '/';
		memcpy(lpath, lib, len+1);
		
		if (!(newlib = dlopen(buf, RTLD_NOW))) {
			return (newlib = dlerror()) ? newlib : dlopen_err;
		}
	}
	handle->lib = newlib;
	handle->create = 0;
	
	return 0;
}

