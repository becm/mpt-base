#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(core.h)

int main(int argc, char *argv[])
{
	int i;
	
	for (i = 1; i < argc; ++i) {
		int val;;
		if (sscanf(argv[i], "%i", &val) != 1) {
			fputs("- ", stdout);
		}
		else {
			fprintf(stdout, "%i ", (int) mpt_identifier_align(val));
		}
	}
	fputc('\n', stdout);
	return 0;
}
