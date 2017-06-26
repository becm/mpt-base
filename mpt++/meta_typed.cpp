/*
 * MPT C++ metatype creator
 */

#include "meta.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptMeta
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
    switch (type) {
      // integer types
      case typeIdentifier<uint8_t>(): return new Metatype<uint8_t>(static_cast<const uint8_t *>(ptr));
      default: return 0;
    }
}

__MPT_NAMESPACE_END