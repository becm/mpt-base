#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(types.h)

#include MPT_INCLUDE(values.h)

extern int main(int argc, char *argv[])
{
	int i;
	
	for (i = 1; i < argc; ++i) {
		MPT_INTERFACE(metatype) *src;
		MPT_INTERFACE(iterator) *it;
		if (!(src = mpt_iterator_create(argv[i]))) {
			fputs("bad format <", stderr);
			fputs(argv[i], stderr);
			fputc('>', stderr);
			fputc('\n', stderr);
			continue;
		}
		MPT_metatype_convert(src, MPT_ENUM(TypeIteratorPtr), &it);
		while (1) {
			const MPT_STRUCT(value) *src;
			double val;
			int res;
			if (!(src = it->_vptr->value(it))) {
				fprintf(stderr, "%s\n", "value query error");
				break;
			}
			if ((res = mpt_value_convert(src, 'd', &val)) < 0) {
				fprintf(stderr, "%s: %d\n", "conversion error", -res);
				break;
			}
			fprintf(stdout, "%g ", val);
			
			if ((res = it->_vptr->advance(it)) < 0) {
				fprintf(stderr, "%s: %d\n", "advance error", -res);
				break;
			}
			if (!res) {
				break;
			}
		}
		src->_vptr->unref(src);
		fputc('\n', stdout);
	}
	return 0;
}
