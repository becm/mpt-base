
#include <stdio.h>
#include <stdlib.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(object.h)

#include MPT_INCLUDE(loader.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

const char *object_name(MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(property) pr;
	pr.name = "";
	if (obj->_vptr->property(obj, &pr) < 0) {
		return 0;
	}
	return pr.name;
}

int main(int argc, const char *argv[])
{
	struct mpt_libsymbol ls = MPT_LIBSYMBOL_INIT;
	struct mpt_metatype *mt;
	const char *sym;
	union {
		void *addr;
		void *(*fcn)(void);
	} ptr;
	
	mtrace();
	
	sym = "mpt_output_local@libmptplot.so.1";
	
	do {
		if (mpt_library_bind(&ls, sym, getenv("MPT_PREFIX_LIB"), 0) < 0) {
			return 1;
		}
		/* create reference with loaded function */
		if ((ptr.addr = ls.addr) && (mt = ptr.fcn())) {
			FILE *out = stdout;
			const uint8_t *types = 0;
			struct mpt_object *obj = 0;
			int err;
			
			if (mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeObject)), &obj) >= 0
			    && obj) {
				fputs(object_name(obj), out);
			}
			if ((err = mt->_vptr->conv(mt, 0, &types)) >= 0) {
				fprintf(out, ": 0x%02x", err);
				if (types) {
					fputs(" >", out);
					while (*types) {
						fprintf(out, " 0x%02x", *types++);
					}
				}
			}
			fputs("\n", out);
			mt->_vptr->instance.unref((void *) mt);
		}
		sym = *(++argv);
	}
	while (--argc);
	mpt_library_detach(&ls.lib);
	return 0;
}
