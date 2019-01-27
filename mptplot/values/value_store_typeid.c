/*!
 *  MPT plot library
 *    register value store with type system
 */

#include "types.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief value store type
 * 
 * Get or register value store data type.
 * 
 * \return value store id
 */
extern int mpt_value_store_typeid(void)
{
	static int id = 0;
	
	if (!id) {
		id = mpt_type_generic_new();
	}
	return id;
}
