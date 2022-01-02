/*
 * MPT C++ convertable operations
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptConvert
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

/*!
 * \ingroup mptConvert
 * \brief get convertable interface traits
 * 
 * Get named traits for convertable pointer data.
 * 
 * \return named traits for convertable pointer
 */
const struct named_traits *convertable::pointer_traits()
{
	return mpt_interface_traits(TypeConvertablePtr);
}

__MPT_NAMESPACE_END
