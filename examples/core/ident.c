
#include <stdio.h>
#include <mpt/core.h>

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
