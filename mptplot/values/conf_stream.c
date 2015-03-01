/*!
 * read matrix data from MPT stream.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <errno.h>

#include "values.h"

extern int mpt_conf_stream(MPT_STRUCT(array) *arr, void *stream, int len, int ld)
{
	/*
	int lmap;
	lmap = map ? ((ld < 0) ? -ld : ld) : 0;
	(void) map;
	*/
	(void) ld;
	
	if (!arr || !stream) {
		errno = EFAULT; return -1;
	}
	if (len < 0) {
		errno = ERANGE; return -2;
	}
	
	return 0;
}


