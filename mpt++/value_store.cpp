/*
 * MPT C++ typed array
 */

#include "message.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

// typed information for array
int value_store::type() const
{
    const array::content *d;
    if (!(d = _d.data())) {
        return BadArgument;
    }
    const type_traits *t;
    if (!(t = d->typeinfo())) {
        return BadType;
    }
    return t->type;
}
size_t value_store::element_size() const
{
    const array::content *d;
    if (!(d = _d.data())) {
        return 0;
    }
    const type_traits *t;
    if (!(t = d->typeinfo())) {
        return 0;
    }
    return t->size;
}
long value_store::element_count() const
{
    const array::content *d;
    if (!(d = _d.data())) {
        return BadArgument;
    }
    const type_traits *t;
    if (!(t = d->typeinfo())) {
        return BadType;
    }
    return d->length() / t->size;
}
void value_store::set_modified(bool set)
{
    if (set) {
        _flags |= ValueChange;
    } else {
        _flags &= ~ValueChange;
    }
}

void *value_store::reserve(int type, size_t len, long off)
{
    return mpt_value_store_reserve(&_d, type, len, off);
}
size_t maxsize(span<const value_store> sl, int type)
{
    const value_store *val = sl.begin();
    size_t len = 0;
    for (size_t i = 0, max = sl.size(); i < max; ++i) {
        if (!val->element_size()) {
            continue;
        }
        if (type >= 0 && type != val->type()) {
            continue;
        }
        long curr = val->element_count();
        if (curr < 0) {
            continue;
        }
        if ((size_t) curr > len) {
            len = curr;
        }
    }
    return len;
}

__MPT_NAMESPACE_END
