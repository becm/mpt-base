/*!
 * clear cycle.
 */

#include "array.h"
#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief clear data segments
 * 
 * Clear memory used by multi-dimension cycle data.
 * 
 * \param cyc cycle descriptor
 */
extern void mpt_cycle_fini(MPT_STRUCT(cycle) *cyc)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(typed_array) *arr;
	size_t len;
	
	if (!(buf = cyc->_d._buf)) {
		return;
	}
	len = buf->used / sizeof(*arr);
	arr = (void *) (buf + 1);
	while (len--) {
		if (!(buf = (arr++)->_d._buf)) {
			continue;
		}
		if (buf->shared) {
			--buf->shared;
			continue;
		}
		if (buf->resize) {
			buf->resize(buf, 0);
		}
	}
}
