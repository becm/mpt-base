/*
 * mpt C++ library
 *   iterator wrappers
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptMeta
 * \brief get iterator interface traits
 * 
 * Get named traits for iterator pointer data.
 * 
 * \return named traits for iterator pointer
 */
const struct named_traits *iterator::pointer_traits()
{
	return mpt_interface_traits(TypeIteratorPtr);
}

__MPT_NAMESPACE_END
