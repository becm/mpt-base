/*!
 *  MPT plot library
 *    traits for value store
 */

#include <string.h>

#include "types.h"

#include "values.h"

static int _value_store_init(void *dest, const void *src)
{
	const MPT_STRUCT(value_store) *from = src;
	MPT_STRUCT(value_store) *to = dest;
	
	memset(to, 0, sizeof(*to));
	
	if (from) {
		int ret;
		if ((ret = mpt_array_clone(&to->_d, &from->_d)) < 0) {
			return ret;
		}
		to->_type  = from->_type;
		to->_flags = from->_flags;
		to->_code  = from->_code;
		return from->_d._buf ? 1 : 0;
	}
	
	return 0;
}
static void _value_store_fini(void *ptr)
{
	MPT_STRUCT(value_store) *val = ptr;
	mpt_array_clone(&val->_d, 0);
}

/*!
 * \ingroup mptValues
 * \brief traits for value store
 * 
 * Get value store operations and size.
 * 
 * \return value store id
 */
extern const MPT_STRUCT(type_traits) *mpt_value_store_traits(void)
{
	static const MPT_STRUCT(type_traits) traits = {
		_value_store_init,
		_value_store_fini,
		sizeof(MPT_STRUCT(value_store))
	};
	
	return &traits;
}
