/*
 * mpt C++ library
 *   type ID classification
 */

#include <cstdlib>

#include "core.h"

__MPT_NAMESPACE_BEGIN

bool value::isScalar(int type)
{
    /* regular and user scalar type */
    return (type &= 0x7f) >= 0x60;
}
bool value::isPointer(int type)
{
    /* remove extra flags */
    type &= 0xff;

    /* builtin pointer types */
    if (type >= 0x8 && type < TypeVecBase) return true;

    /* no further pointers in base types */
    if (type < MPT_ENUM(TypeUser)) return false;

    /* user registered pointer */
    return type < (MPT_ENUM(TypeUser) + TypeVecBase);
}
bool value::isVector(int type)
{
    /* remove extra flags */
    type &= 0x7f;

    /* check vector type range */
    return (type >= TypeVecBase && type < TypeArrBase);
}
bool value::isArray(int type)
{
    /* remove extra flags */
    type &= 0x7f;

    /* check array type range */
    return (type >= TypeArrBase && type < TypeScalBase);
}

__MPT_NAMESPACE_END
