/*!
 * clear cycle.
 */

#include <string.h>

#include "array.h"
#include "types.h"

#include "values.h"

static int _rawdata_stage_init(void *ptr, const void *src)
{
	const MPT_STRUCT(rawdata_stage) *from = src;
	MPT_STRUCT(rawdata_stage) *to = ptr;
	int ret = 0;
	
	memset(to, 0, sizeof(*to));
	
	if (from) {
		if ((ret = mpt_array_clone(&to->_d, &from->_d)) < 0) {
			return ret;
		}
		to->_max_dimensions = from->_max_dimensions;
	}
	return ret;
}

static void _rawdata_stage_fini(void *ptr)
{
	MPT_STRUCT(rawdata_stage) *st = ptr;
	mpt_array_clone(&st->_d, 0);
	st->_max_dimensions = 0;
}

/*!
 * \ingroup mptPlot
 * \brief traits for rawdata stage
 * 
 * Get rawdata stage operations and size.
 * 
 * \return rawdata stage traits
 */
extern const MPT_STRUCT(type_traits) *mpt_stage_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_rawdata_stage_init,
		_rawdata_stage_fini,
		sizeof(MPT_STRUCT(rawdata_stage))
	};
	
	return &traits;
}
