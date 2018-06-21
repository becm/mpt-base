/*
 * MPT C++ metatype creator
 */

#include "meta.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptMeta
 * \brief get pointer element
 * 
 * Convert metatype to valid pointer
 * 
 * \param type  storage data type
 * 
 * \return converted pointer
 */
void *metatype::pointer(int type) const
{
    if (!type) {
        type = conv(0, 0);
    }
    if (mpt_valsize(type) != 0) {
        return 0;
    }
    void *ptr = 0;
    if (conv(type, &ptr) < 0) {
        return 0;
    }
    return ptr;
}
/*!
 * \ingroup mptMeta
 * \brief get string data
 * 
 * Convert metatype to string data
 * 
 * \return C string base pointer
 */
const char *metatype::string() const
{
    return mpt_meta_data(this, 0);
}
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
        case typeinfo<uint8_t>::id(): return new meta_value<uint8_t>(static_cast<const uint8_t *>(ptr));
      default: return 0;
    }
}

__MPT_NAMESPACE_END
