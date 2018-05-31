/*!
 * clear cycle.
 */

#include "array.h"

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief clear stage data segments
 * 
 * Clear memory used by multi-dimension raw data.
 * 
 * \param st raw data stage
 */
extern void mpt_stage_fini(MPT_STRUCT(rawdata_stage) *st)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(value_store) *val;
	size_t len;
	
	if (!(buf = st->_d._buf)) {
		return;
	}
	len = buf->_used / sizeof(*val);
	val = (void *) (buf + 1);
	while (len--) {
		mpt_array_clone(&(val++)->_d, 0);
	}
	mpt_array_clone(&st->_d, 0);
}
