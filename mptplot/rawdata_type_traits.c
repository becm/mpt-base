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
extern const MPT_STRUCT(named_traits) *mpt_rawdata_type_traits(void)
{
	const MPT_STRUCT(named_traits) *traits = 0;
	if (!traits) {
		traits = mpt_type_interface_add("mpt.rawdata");
	}
	return traits;
}
