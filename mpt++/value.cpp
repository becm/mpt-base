/*
 * mpt C++ library
 *   type ID classification
 */

#include "array.h"

#include "convert.h"

__MPT_NAMESPACE_BEGIN

// conversion wrapper
int convert(const void **val, int fmt, void *dest, int type)
{
    return mpt_data_convert(val, fmt, dest, type);
}

// byte order adaption
float swapOrder(float v)
{
    mpt_bswap_32(1, reinterpret_cast<uint32_t *>(&v));
    return v;
}
double swapOrder(double v)
{
    mpt_bswap_64(1, reinterpret_cast<uint64_t *>(&v));
    return v;
}
float80 swapOrder(float80 v)
{
    mpt_bswap_80(1, &v);
    return v;
}

// transport value operations
float80 &float80::operator= (long double val)
{
    mpt_float80_encode(1, &val, this);
    return *this;
}
long double float80::value() const
{
    long double v = 0;
    mpt_float80_decode(1, this, &v);
    return v;
}

size_t value::scalar(int type)
{
    /* incompatible type */
    if (!fmt || type < 0 || type > _TypeFinal) {
        return 0;
    }
    /* exact scalar type */
    if (type && *fmt != type) {
        return 0;
    }
    /* regular and user scalar type */
    ssize_t s;
    if ((s = mpt_valsize(type)) <= 0) {
        return 0;
    }
    return s;
}
void * const *value::pointer(int type)
{
    /* incompatible type */
    if (!ptr || !fmt || (type && type != *fmt)) {
        return 0;
    }
    if (mpt_valsize(type)) {
        return 0;
    }
    return reinterpret_cast<void * const *>(ptr);
}
const struct iovec *value::vector(int type)
{
    int from;
    if (!ptr || !fmt || !(from = *fmt)) {
        return 0;
    }
    if (!MPT_value_isVector(from)) {
        return 0;
    }
    /* type out of range */
    if (type && from != !MPT_value_toVector(type)) {
        return 0;
    }
    return reinterpret_cast<const struct iovec *>(ptr);
}
const array *value::array(int type)
{
    int from;
    if (!ptr || !fmt || !(from = *fmt)) {
        return 0;
    }
    /* type out of range */
    if (from != MPT_ENUM(TypeBuffer)) {
        return 0;
    }
    const struct array *arr = reinterpret_cast<const struct array *>(ptr);
    buffer *b = arr->ref().pointer();
    if (type && b && type != b->content()) {
        return 0;
    }
    return arr;
}

__MPT_NAMESPACE_END
