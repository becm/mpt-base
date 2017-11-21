
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "../module.h"

#include MPT_INCLUDE(client.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	MPT_STRUCT(libhandle) lh = MPT_LIBHANDLE_INIT;
	MPT_STRUCT(module_generic) *mod;
	const char *err;
	
	mtrace();
	
	if ((err = mpt_library_assign(&lh, "mpt_output_local@libmptio.so.1", getenv("MPT_PREFIX_LIB")))) {
		warn("%s", err);
		return 1;
	}
	if ((mod = lh.create())) {
		MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
		pr.name = "";
		mod->_obj._vptr->property(&mod->_obj, &pr);
		puts(pr.name);
		puts(pr.desc);
		mod->_mt._vptr->ref.unref((void *) mod);
	}
	return 0;
}
 
