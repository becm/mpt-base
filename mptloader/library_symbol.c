/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <string.h>

#include <dlfcn.h>

#include "loader.h"

/*!
 * \ingroup mptLoader
 * \brief assign library symbol
 * 
 * Connect to symbol in library.
 * 
 * \param sym    library symbol descriptor
 * \param descr  symbol (@lib) to bind
 * 
 * \return dynamic loader error message
 */
extern int mpt_library_symbol(MPT_STRUCT(libsymbol) *sym, const char *descr)
{
	const char *libname;
	void *addr = 0;
	void *lib = 0;
	
	if (sym->lib) {
		lib = sym->lib->addr;
	}
	if (!descr) {
		dlerror();
		return MPT_ERROR(BadArgument);
	}
	if (!(libname = strchr(descr, '@'))
	    || !*libname
	    || isspace(*libname)) {
		addr = dlsym(lib, descr);
	}
	else {
		char buf[128];
		size_t len;
		if ((len = libname++ - descr) >= sizeof(buf)) {
			dlerror();
			return MPT_ERROR(MissingBuffer);
		}
		if (!len) {
			dlerror();
			return MPT_ERROR(MissingBuffer);
		}
		descr = memcpy(buf, descr, len);
		buf[len] = '\0';
		addr = dlsym(lib, descr);
	}
	if (!addr) {
		return MPT_ERROR(BadValue);
	}
	sym->addr = addr;
	sym->type = 0;
	
	return 0;
}
