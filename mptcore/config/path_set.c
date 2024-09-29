
#include <string.h>
#include <errno.h>

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief set path data
 * 
 * Set path data to value.
 * 
 * \param path path information
 * \param val  path value
 * \param len  value length
 * 
 * \return number of path elements
 */
extern int mpt_path_set(MPT_STRUCT(path) *path, const char *val, int len)
{
	size_t vlen, plen = 0, first = 0, add = 1;
	int elem = 0;
	char sep, assign;
	
	sep = path->sep;
	assign = path->assign;
	
	if (!val) {
		vlen = add = 0;
	} else {
		vlen = (len < 0) ? strlen(val) + 1 : (size_t) len;
	}
	
	while (plen < vlen) {
		char curr = val[plen++];
		
		/* inline assignment */
		if (curr == assign) {
			add = 0;
			++elem;
			break;
		}
		/* separator found, save length of first part */
		if ((curr == sep) && !elem++) {
			first = plen - 1;
		}
	}
	mpt_path_fini(path);
	
	path->base = val;
	path->off  = 0;
	path->len  = plen + add;
	
	path->first  = (first > UINT8_MAX) ? 0 : first;
	path->sep    = sep;
	path->assign = assign;
	path->flags  = 0;
	
	return elem;
}
