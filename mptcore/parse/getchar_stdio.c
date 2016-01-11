/*!
 * (un)buffered read operation from file descriptor.
 */

#include <stdio.h>

#include "parse.h"

extern int mpt_getchar_stdio(FILE *fd)
{
	int ret;
	
	if ((ret = fgetc(fd)) == EOF) {
		return feof(fd) ? -2 : -1;
	}
	return ret;
}

