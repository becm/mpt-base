
#include "array.h"
#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief set array sizes
 * 
 * Set used size of all arrays to same size.
 * Use longest element size if len < 0.
 * 
 * \return (new) length of modified arrays
 */
extern ssize_t mpt_pline_truncate(MPT_STRUCT(array) *pt, size_t max, ssize_t len)
{
	size_t i;
	
	if (len < 0) {
		len = 0;
		for (i = 0; i < max; i++) {
			MPT_STRUCT(buffer) *buf = pt[i]._buf;
			ssize_t curr = buf ? buf->used : 0;
			if (curr > len) len = curr;
		}
	}
	for (i = 0; i < max; i++) {
		if (!len) {
			if (!pt[i]._buf) continue;
		}
		else if (!mpt_array_slice(pt+i, 0, len)) {
			return -1;
		}
		pt[i]._buf->used = len;
	}
	return len;
}

