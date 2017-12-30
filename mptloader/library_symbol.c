/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <dlfcn.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief assign library handle
 * 
 * Connect file handle to symbol in library.
 * 
 * \param lib    library handle
 * \param descr  symbol (@lib) to open
 * 
 * \return dynamic loader error message
 */
extern void *(*mpt_library_symbol(void *lib, const char *descr))(void)
{
	const char *libname;
	union {
		void *ptr;
		void *(*fcn)(void);
	} tmp;
	
	if (!descr) {
		dlerror();
		errno = EINVAL;
		return 0;
	}
	if (!(libname = strchr(descr, '@'))
	    || !*libname
	    || isspace(*libname)) {
		tmp.ptr = dlsym(lib, descr);
	}
	else {
		char buf[128];
		size_t len;
		if ((len = libname++ - descr) >= sizeof(buf)) {
			dlerror();
			errno = ERANGE;
			return 0;
		}
		if (!len) {
			dlerror();
			errno = EINVAL;
			return 0;
		}
		descr = memcpy(buf, descr, len);
		buf[len] = '\0';
		tmp.ptr = dlsym(lib, descr);
	}
	return tmp.fcn;
}
