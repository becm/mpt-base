

#include <stdio.h>

#include <mpt/values.h>

extern int main(int argc, char *argv[])
{
	int i;
	
	for (i = 1; i < argc; ++i) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = mpt_iterator_create(argv[i]))) {
			fputs("bad format <", stderr);
			fputs(argv[i], stderr);
			fputc('>', stderr);
			fputc('\n', stderr);
			continue;
		}
		while (1) {
			double val;
			int res;
			if (!(res = mt->_vptr->conv(mt, 'd' | MPT_ENUM(ValueConsume), &val))) {
				break;
			}
			if (res < 0) {
				fprintf(stderr, "%s: %d\n", "conversion error", -res);
				break;
			}
			fprintf(stdout, "%g ", val);
		}
		mt->_vptr->unref(mt);
		fputc('\n', stdout);
	}
	
	return 0;
}
