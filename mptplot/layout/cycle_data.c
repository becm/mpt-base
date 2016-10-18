/*!
 * get raw data type and data area.
 */

#include <errno.h>

#include "array.h"
#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief get/create 
 * 
 * Set flags and get target address for raw data slice.
 * 
 * \param cyc  raw data descriptor
 * \param dim  dimension to process
 * \param fmt  slice data type and flags
 * 
 * \return dimension data array
 */
extern MPT_STRUCT(typed_array) *mpt_cycle_data(MPT_STRUCT(cycle) *cyc, unsigned dim, int flg)
{
	MPT_STRUCT(buffer) *buf = cyc->_d._buf;
	MPT_STRUCT(typed_array) *arr;
	
	/* reuse existing dimension */
	if (buf && buf->used/sizeof(*arr) < dim) {
		arr = (void *) (buf+1);
		arr += dim;
		
		if (flg < 0) {
			return arr;
		}
		flg &= ~MPT_ENUM(ValueCreate);
		
		if (flg & MPT_ENUM(ValueReset)) {
			flg &= ~MPT_ENUM(ValueReset);
			arr->_esize  = 0;
			arr->_format = 0;
		}
		arr->_flags = flg;
		return arr;
	}
	/* not allowed to extend dimensions */
	if (!(flg & MPT_ENUM(ValueCreate))) {
		errno = EINVAL;
		return 0;
	}
	if (!(arr = mpt_array_insert(&cyc->_d, dim * sizeof(*arr), 0))) {
		return arr;
	}
	arr->_flags  = flg;
	arr->_esize  = 0;
	arr->_format = 0;
	
	return arr;
}
