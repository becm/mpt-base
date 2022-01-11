#include <stdio.h>
#include <stdlib.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(core.h)

int main(int argc, char *argv[])
{
	int ret = 0, i;
	
	for (i = 1; i < argc; ++i) {
		MPT_STRUCT(identifier) *id;
		int val;
		if (sscanf(argv[i], "%i", &val) != 1) {
			fputs("- ", stdout);
			continue;
		}
		if (!(id = mpt_identifier_new(val))) {
			fputs("(err)", stdout);
			++ret;
			continue;
		}
		fprintf(stdout, "%i_%i ", val, 4 + id->_max);
		mpt_identifier_set(id, 0, 0);
		free(id);
	}
	fputc('\n', stdout);
	return ret;
}
