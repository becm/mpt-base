/*
 * MPT C++ metatype creator
 */

#include "types.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN
/*!
 * \ingroup mptType
 * \brief create typed metatype
 * 
 * Create and initialize Metatype class of specific type
 * 
 * \param type  storage data type
 * \param val   initial metatype value
 * 
 * \return new metatype
 */
metatype *metatype::create(int type, const void *ptr)
{
	// integer types
	if (type == type_properties<uint8_t>::id()) {
		return new meta_value<uint8_t>(static_cast<const uint8_t *>(ptr));
	}
	return 0;
}

__MPT_NAMESPACE_END
