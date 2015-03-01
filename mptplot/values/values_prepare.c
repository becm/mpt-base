
#include <string.h>
#include <errno.h>

#include "array.h"

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
extern double *mpt_values_prepare(MPT_STRUCT(array) *arr, int len)
{
	double	*data;
	size_t	used, add;
	
	if (!len) return 0;
	
	if (len > 0) return mpt_array_append(arr, len*sizeof(*data), 0);
	
	add = sizeof(*data) * (-len);
	
	/* not enough data available */
	if (!arr->_buf || ((used = arr->_buf->used) < add)) {
		errno = ERANGE; return 0;
	}
	/* data area at end for old and new data */
	if (!(data = mpt_array_insert(arr, used, add))) {
		return 0;
	}
	/* copy data from old location */
	return memcpy(data, data+len, add);
}

