
#include <stdlib.h>
#include <stdio.h>

#include <mpt/convert.h>

#include <mpt/values.h>

static void fail(const char *txt)
{
	fputs("bad format <", stderr);
	fputs(txt, stderr);
	fputc('>', stderr);
	fputc('\n', stderr);
}

extern int main(int argc, char *argv[])
{
	double *val;
	int i, max;
	
	val = 0;
	max = 0;
	for (i = 1; i < argc; ++i) {
		const char *desc = argv[i];
		int i, len, pos;
		
		if ((pos = mpt_cint(&len, desc, 0, 0)) < 0 || len < 1) {
			fail(desc);
			continue;
		}
		if (len > max) {
			if (!(val = realloc(val, len * sizeof(*val)))) {
				return 1;
			}
			max = len;
		}
		desc += pos;
		
		if ((pos = mpt_valtype_select(&desc)) < 0
		    || (pos = mpt_valtype_init(pos, desc, len, val, 1, 0)) < 0) {
			fail(desc);
			continue;
		}
		for (i = 0; i < len; ++i) fprintf(stdout, "%g ", val[i]);
		fputc('\n', stdout);
	}
	return 0;
}