/*
 * MPT C++ type identifier
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptCore
 * \brief get pointer id
 * 
 * Convert to type code of pointer element.
 * 
 * \param from  base type
 * 
 * \retval <0 conversion error
 * \retval >0 item type code
 */
extern int to_pointer_id(int from)
{
    if (from < 0) {
        return BadValue;
    }
    if (from > _TypeGenericMax) {
        return BadType;
    }
    return _TypePointerBase + from;
}
/*!
 * \ingroup mptCore
 * \brief get reference id
 * 
 * Convert to type code of reference element.
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
 * \brief get item id
 * 
 * Convert to type code of item element.
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
 * To register explicit metatype, interface or base type
 * use supplied special functions.
 * 
 * \return new type code
 */
extern int make_id()
{
    return mpt_type_generic_new();
}
/*!
 * \ingroup mptCore
 * \brief get span id
 * 
 * Convert to type code vector range.
 * 
 * \return new type code
 */
extern int to_span_id(int from)
{
    if (from < 0) {
        return BadValue;
    }
    if (from >= _TypeSpanBase) {
        return BadType;
    }
    if (MPT_type_isScalar(from) || MPT_type_isExtended(from)) {
        return from + _TypeVectorBase - _TypeScalarBase;
    }
    return _TypeSpanBase + from;
}

__MPT_NAMESPACE_END
