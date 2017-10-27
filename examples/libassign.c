
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
	struct mpt_metatype *mt;
	const char *str;
	
	mtrace();
	
	/* load I/O library and assign symbol */
	if ((str = mpt_library_assign(&lh, "mpt_output_local@libmptio.so.1", getenv("MPT_PREFIX_LIB")))) {
		fputs(str, stderr);
		return 1;
	}
	/* create reference with loaded function */
	if ((mt = lh.create())) {
		struct mpt_object *obj = 0;
		mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj);
		printf("%s: 0x%02x\n", "object type", obj->_vptr->property(obj, 0));
		puts(mpt_object_typename(obj));
		mt->_vptr->ref.unref((void *) mt);
	}
	mpt_library_close(&lh);
	return 0;
}
