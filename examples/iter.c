#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(convert.h)

#include MPT_INCLUDE(values.h)

extern int main(int argc, char *argv[])
{
	int i;
	
	for (i = 1; i < argc; ++i) {
		MPT_INTERFACE(iterator) *it;
		
		if (!(it = mpt_iterator_create(argv[i]))) {
			fputs("bad format <", stderr);
			fputs(argv[i], stderr);
			fputc('>', stderr);
			fputc('\n', stderr);
			continue;
		}
		while (1) {
			double val;
			int res;
			if (!(res = it->_vptr->meta.conv((void *) it, 'd', &val))) {
				fprintf(stderr, "%s: %d\n", "conversion range failed", -res);
				break;
			}
			if (res < 0) {
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
		it->_vptr->meta.ref.unref((void *) it);
		fputc('\n', stdout);
	}
	return 0;
}
