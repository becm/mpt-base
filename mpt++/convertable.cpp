/*
 * MPT C++ convertable operations
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptType
 * \brief get pointer element
 * 
 * Convert value to valid pointer
 * 
 * \param type  storage data type
 * 
 * \return converted pointer
 */
void *convertable::pointer(int type)
{
	if (!type) {
		type = convert(0, 0);
	}
	if (mpt_valsize(type) != 0) {
		return 0;
	}
	void *ptr = 0;
	if (convert(type, &ptr) < 0) {
		return 0;
	}
	return ptr;
}
/*!
 * \ingroup mptType
 * \brief get string data
 * 
 * Convert value to string data
 * 
 * \return C string base pointer
 */
const char *convertable::string()
{
	return mpt_convertable_data(this, 0);
}

__MPT_NAMESPACE_END
