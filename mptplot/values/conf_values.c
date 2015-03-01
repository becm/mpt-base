
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "array.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief append formated data
 * 
 * Append data described by string argument
 * 
 * \param arr   target array
 * \param len   number of elements to add
 * \param descr string describing values
 * 
 * \return appended data
 */
extern double *mpt_conf_values(MPT_STRUCT(array) *arr, int len, const char *descr)
{
	double *data;
	int type;
	
	/* enshure usage of "len" additional elements */
	if (!(data = mpt_values_prepare(arr, len)) || !descr)
		return data;
	
	/* leave data initialized to zero */
	if ((type = mpt_valtype_select(descr, (char **) &descr)) >= 0) {
		if ((type = mpt_valtype_init(len, data, 1, descr, type, data)) >= 0) {
			return data;
		}
	}
	arr->_buf->used -= len * sizeof(double);
	return 0;
}

