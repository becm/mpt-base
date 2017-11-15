/*!
 * register input with type system.
 */

#include "notify.h"


/*!
 * \ingroup mptNotify
 * \brief get input type
 * 
 * Get or register input reference type.
 * 
 * \return input id
 */
extern int mpt_input_typeid(void)
{
	static int id = 0;
	
	if (!id) {
		id = mpt_valtype_meta_new("input");
	}
	return id;
}
