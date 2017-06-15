/*!
 * generate values.
 */

#include <string.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief reserve data on array
 * 
 * Prepare new elements on array.
 * Copy old data if length arument is less than zero.
 * 
 * \param arr  array to append data
 * \param len  size to append
 * 
 * \return zeroed/copied start address
 */
extern double *mpt_values_prepare(_MPT_ARRAY_TYPE(double) *arr, int len)
{
	MPT_STRUCT(buffer) *buf;
	double *data;
	long used, add;
	
	if (!(buf = arr->_buf)) {
		if (len < 0) {
			errno = EINVAL;
			return 0;
		}
		return mpt_array_append(arr, len * sizeof(*data), 0);
	}
	if (!(used = buf->_vptr->content(buf))) {
		len /= sizeof(*data);
	}
	else if (used != 'd') {
		errno = EINVAL;
		return 0;
	}
	used = buf->_used / sizeof(*data);
	if ((add = len) < 0) {
		add = -len;
		if (used < add) {
			errno = EINVAL;
			return 0;
		}
	}
	if (!(buf = buf->_vptr->detach(buf, used + add))) {
		return 0;
	}
	used *= sizeof(*data);
	add *= sizeof(*data);
	/* data area at end for new data */
	if (!(data = mpt_buffer_insert(buf, used, add))) {
		return 0;
	}
	/* copy data from old location */
	if (len < 0) {
		memcpy(data, data + len, add);
	}
	return data;
}

