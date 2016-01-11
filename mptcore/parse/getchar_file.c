/*!
 * (un)buffered read operation from file descriptor.
 */

#include <unistd.h>

#include "parse.h"

extern int mpt_getchar_file(void *file)
{
	uint8_t curr;
	uintptr_t err = (uintptr_t) file;
	
	if ((err = read(err, &curr, 1)) == 1) {
		return curr;
	}
	return err ? -1 : -2;
}

