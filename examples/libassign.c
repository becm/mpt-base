
#include <stdio.h>
#include <stdlib.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(object.h)
#include MPT_INCLUDE(client.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	struct mpt_libhandle lh = MPT_LIBHANDLE_INIT;
	struct mpt_object *out;
	const char *str;
	
	mtrace();
	
	/* load I/O library and assign symbol */
	if ((str = mpt_library_assign(&lh, "mpt_output_local@libmptio.so.1", getenv("MPT_PREFIX_LIB")))) {
		fputs(str, stderr);
		return 1;
	}
	/* create reference with loaded function */
	if ((out = lh.create())) {
		printf("%s: 0x%02x\n", "object type", out->_vptr->property(out, 0));
		puts(mpt_object_typename((void *) out));
		out->_vptr->ref.unref((void *) out);
	}
	mpt_library_close(&lh);
	return 0;
}
