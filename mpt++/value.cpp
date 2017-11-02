/*
 * mpt C++ library
 *   type ID classification
 */

#include <sys/uio.h>

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

bool value::set(const char *f, const void *d)
{
    if (!f || !d) {
        return false;
    }
    fmt = f;
    ptr = d;
    return true;
}

const char *value::string() const
{
    
    if (!fmt) {
        return static_cast<const char *>(ptr);
    }
    if (!*fmt) {
        return 0;
    }
    const void *str = ptr;
    return mpt_data_tostring(&str, *fmt, 0);
}
const void *value::scalar(int type) const
{
    /* incompatible type */
    int from;
    if (!ptr || !fmt || type < 0 || !(from = (uint8_t) *fmt)) {
        return 0;
    }
    /* exact scalar type */
    if (type && type != from) {
        return 0;
    }
    /* regular and user scalar type */
    if (type > _TypeDynamicMax
        || mpt_valsize(from) <= 0) {
          return 0;
    }
    return ptr;
}
void *value::pointer(int type) const
{
    int from;
    /* incompatible type */
    if (!ptr || !fmt || type < 0 || !(from = (uint8_t) *fmt)) {
        return 0;
    }
    if (type && type != from) {
        return 0;
    }
    if (from <= _TypeDynamicMax
        && mpt_valsize(from) != 0) {
        return 0;
    }
    return *reinterpret_cast<void * const *>(ptr);
}
const struct iovec *value::vector(int type) const
{
    int from;
    if (!ptr || !fmt || type < 0 || !(from = (uint8_t) *fmt)) {
        return 0;
    }
    /* bad source type */
    if ((from = MPT_value_fromVector(from)) < 0) {
        return 0;
    }
    /* invald content type */
    if (type && type != from) {
        return 0;
    }
    return reinterpret_cast<const struct iovec *>(ptr);
}
const array *value::array(int type) const
{
    int from;
    if (!ptr || !fmt || type < 0 || !(from = (uint8_t) *fmt)) {
        return 0;
    }
    /* type out of range */
    if (from != MPT_ENUM(TypeArray)) {
        return 0;
    }
    const struct array *arr = reinterpret_cast<const struct array *>(ptr);
    const buffer *b = arr->data();
    if (type >= 0 && type != b->content()) {
        return 0;
    }
    return arr;
}

__MPT_NAMESPACE_END
