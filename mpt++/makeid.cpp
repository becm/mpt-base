/*
 * MPT C++ type identifier
 */

#include "core.h"

enum {
	TypeAuto    = 0x10000,
	ItemStatic  = 2 * TypeAuto,
	ItemDynamic = 3 * TypeAuto,
	ItemLimit   = 4 * TypeAuto - 1
};

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptCore
 * \brief create type id
 * 
 * Convert or create type code for item element.
 * 
 * \param type  base type
 * 
 * \return item type code
 */
extern int makeItemId(int from)
{
    if (from < 0) {
        static int id = 0;
        if (id >= ItemLimit) {
            return BadValue;
        }
        return ItemDynamic + id++;
    }
    if (from >= ItemStatic) {
        return BadType;
    }
    return ItemStatic + from;
}
/*!
 * \ingroup mptCore
 * \brief create type id
 * 
 * Convert or create type code for item element.
 * 
 * \return item type code
 */
extern int makeId()
{
    static int id = TypeAuto;
    if (id >= ItemStatic) {
        return BadValue;
    }
    return id++;
}

__MPT_NAMESPACE_END
