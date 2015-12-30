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
    if (type >= 0x8 && type < 0x20) return true;

    /* no further pointers in base types */
    if (type < MPT_ENUM(TypeUser)) return false;

    /* user registered pointer */
    return type < (MPT_ENUM(TypeUser) + 0x20);
}
bool value::isVector(int type)
{
    /* remove extra flags */
    type &= 0x7f;

    /* check vector type range */
    return (type >= 0x20 && type < 0x40);
}
bool value::isArray(int type)
{
    /* remove extra flags */
    type &= 0x7f;

    /* check array type range */
    return (type >= 0x40 && type < 0x60);
}
static char *vectorFormats = 0;
const char *value::vectorFormat(int type)
{
    if (!isScalar(type)) return 0;
    if (!vectorFormats) {
        if (!(vectorFormats = (char *) std::malloc(0x40))) return 0;
        for (int i = 0; i < 0x20; ++i) {
            vectorFormats[i*2] = i + 0x20;
            vectorFormats[i*2+1] = 0;
        }
    }
    return vectorFormats + 2 * ((type & 0xff) - 0x40);
}

__MPT_NAMESPACE_END
