/*!
 * register input with type system.
 */

#include "values.h"


/*!
 * \ingroup mptNotify
 * \brief get input type
 * 
 * Get or register input reference type.
 * 
 * \return input id
 */
extern int mpt_value_store_typeid(void)
{
	static int id = 0;
	
	if (!id) {
		/* TODO: use type registration when avail in core */
		id = 0xc0fefe;
	}
	return id;
}
