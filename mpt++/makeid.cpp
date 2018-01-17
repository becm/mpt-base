/*
 * MPT C++ type identifier
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptCore
 * \brief create type id
 * 
 * Convert type code for reference element.
 * 
 * \param from  base type
 * 
 * \retval <0 conversion error
 * \retval >0 item type code
 */
extern int to_reference_id(int from)
{
    if (from < 0) {
        return BadValue;
    }
    if (from > _TypeGenericMax) {
        return BadType;
    }
    return _TypeReferenceBase + from;
}
/*!
 * \ingroup mptCore
 * \brief create type id
 * 
 * Convert type code for item element.
 * 
 * \param from  base type
 * 
 * \retval <0 conversion error
 * \retval >0 item type code
 */
extern int to_item_id(int from)
{
    if (from < 0) {
        return BadValue;
    }
    if (from > _TypeGenericMax) {
        return BadType;
    }
    return _TypeItemBase + from;
}
/*!
 * \ingroup mptCore
 * \brief new generic type
 * 
 * Create new type code in dynamic extended range.
 * To register explicit metatype, interface or
 * dynamic base type use supplied special functions.
 * 
 * \return new type code
 */
extern int make_id()
{
    static int id = _TypeGenericBase;
    if (id >= _TypeGenericMax) {
        return BadValue;
    }
    return id++;
}
/*!
 * \ingroup mptCore
 * \brief new vector type
 * 
 * Create new vector type code in dynamic extended range.
 * 
 * \return new type code
 */
extern int make_vector_id()
{
    // redirect to generic registration
    return make_id();
}

__MPT_NAMESPACE_END
