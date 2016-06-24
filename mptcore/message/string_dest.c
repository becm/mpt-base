/*!
 * modify (initialized) output binding from string
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "message.h"

/*!
 * \ingroup mptOutput
 * \brief get destination
 * 
 * Parse destination elements in (0,255) range.
 * 
 * \param addr  parsed destination
 * \param sep   decoded message data
 * 
 * \return total size of message data
 */
extern int mpt_string_dest(MPT_STRUCT(strdest) *addr, int sep, const char *descr)
{
	char *end;
	size_t pos = 0, max, i;
	long val;
	
	if (!(max = addr->change)) {
		max = sizeof(addr->val);
	}
	else if (max > sizeof(addr->val)) {
		errno = ERANGE; return -1;
	}
	addr->change = 0;
	
	if (!descr)
		return 0;
	
	while (isspace(*descr)) {
		pos++; descr++;
	}
	
	for (i = 0 ; i < max ; i++) {
		val = strtol(descr, &end, 0);
		if (end == descr) {
			if (!*end)
				break;
			
			if (sep && *end == sep) {
				descr++; pos++;
				if (isspace(end[1]))
					break;
				continue;
			}
			errno = EINVAL; return -pos;
		}
		pos += (end - descr);
		
		if (val < 0 || val > UINT8_MAX) {
			errno = ERANGE; return -pos;
		}
		addr->val[i] = val;
		addr->change |= 1 << i;
		
		if (!*(descr = end) || isspace(*descr) || sep != *descr)
			break;
		
		descr++; pos++;
	}
	return pos;
}
