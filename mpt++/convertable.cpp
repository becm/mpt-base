/*
 * MPT C++ convertable operations
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

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
