/*!
 * raw data interface registration.
 */

#include "types.h"

#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief raw data interface
 * 
 * Get or register raw data interface type.
 * 
 * \return raw data type
 */
extern int mpt_rawdata_typeid(void)
{
	static int id = 0;
	if (!id) {
		id = mpt_type_interface_new("rawdata");
	}
	return id;
}
