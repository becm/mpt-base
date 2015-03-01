/*!
 * return zero-terminated data in queue.
 */

#include "queue.h"

extern char *mpt_queue_string(MPT_STRUCT(queue) *qu)
{
	char *str;
	size_t rem;
	
	rem = qu->max - qu->len;
	
	if (!rem) {
		return 0;
	}
	if (rem <= qu->off) {
		mpt_queue_align(qu, 0);
	}
	str = ((char *) qu->base) + qu->off;
	
	str[qu->len] = '\0';
	
	return str;
}
