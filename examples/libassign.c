
#include <stdio.h>
#include <stdlib.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(object.h)
#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(client.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main(int argc, const char *argv[])
{
	struct mpt_libhandle lh = MPT_LIBHANDLE_INIT;
	struct mpt_metatype *mt;
	const char *sym;
	
	mtrace();
	
	/* load I/O library and assign symbol */
	lh.type = MPT_ENUM(TypeMeta);
	
	sym = "mpt_output_local@libmptplot.so.1";
	
	do {
		if (mpt_library_bind(&lh, sym, getenv("MPT_PREFIX_LIB"), 0) < 0) {
			return 1;
		}
		/* create reference with loaded function */
		if ((mt = lh.create())) {
			FILE *out = stdout;
			const uint8_t *types = 0;
			struct mpt_object *obj = 0;
			int err;
			
			if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
			    && obj) {
				fputs(mpt_object_typename(obj), out);
			}
			if ((err = mt->_vptr->conv(mt, 0, &types)) >= 0) {
				fprintf(out, ": 0x%02x", err);
				if (types) {
					fputs(" >", out);
				}
				while (*types) {
					fprintf(out, " 0x%02x", *types++);
				}
			}
			fputs(mpt_newline_string(0), out);
			mt->_vptr->ref.unref((void *) mt);
		}
		sym = *(++argv);
	}
	while (--argc);
	mpt_library_close(&lh);
	return 0;
}
