/*!
 * get raw data type and data area.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief get/create stage dimension
 * 
 * Set flags and get target array for raw data dimension.
 * 
 * \param st   raw data stage data
 * \param dim  dimension to process
 * 
 * \return dimension data array
 */
extern MPT_STRUCT(value_store) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *st, unsigned dim)
{
	MPT_STRUCT(buffer) *buf = st->_d._buf;
	const MPT_STRUCT(type_traits) *traits;
	MPT_STRUCT(value_store) *val;
	unsigned max = 0, used = 0;
	
	/* not allowed to extend dimensions */
	if (st->_max_dimensions
	    && (dim >= st->_max_dimensions)) {
		errno = ENOMEM;
		return 0;
	}
	/* array element traits for value store */
	if (!(traits = mpt_value_store_traits())) {
		errno = EBADSLT;
		return 0;
	}
	/* existing buffer must be valied */
	if (buf) {
		if (traits != buf->_content_traits) {
			errno = EBADSLT;
			return 0;
		}
		used = buf->_used / sizeof(*val);
		max  = used;
	}
	if (dim < used) {
		max = used * sizeof(*val);
	} else {
		max = (dim + 1) * sizeof(*val);
	}
	if (!buf) {
		if (!(buf = _mpt_buffer_alloc(max))) {
			return 0;
		}
		buf->_content_traits = traits;
		if (!(val = mpt_buffer_insert(buf, dim * sizeof(*val), sizeof(*val)))) {
			buf->_vptr->unref(buf);
			return 0;
		}
		if ((traits->init(val, 0) < 0)) {
			errno = ENOTSUP;
			buf->_vptr->unref(buf);
			return 0;
		}
		st->_d._buf = buf;
		return val;
	}
	if (!(buf = buf->_vptr->detach(buf, max))) {
		return 0;
	}
	st->_d._buf = buf;
	
	if (dim < used) {
		val = (void *) (buf + 1);
		return val + dim;
	}
	/* execute init function for inserted data */
	if ((val = mpt_buffer_insert(buf, dim * sizeof(*val), sizeof(*val)))) {
		traits->init(val, 0);
	}
	return val;
}
