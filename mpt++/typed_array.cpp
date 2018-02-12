/*
 * MPT C++ typed array
 */

#include "values.h"

__MPT_NAMESPACE_BEGIN

// typed information for array
bool typed_array::setType(int t)
{
    int esize;
    if ((esize = mpt_valsize(t)) <= 0) {
        return false;
    }
    _type = t;
    _esize = esize;
    return true;
}
void typed_array::setModified(bool set)
{
    if (set) _flags |= ValueChange;
    else _flags &= ~ValueChange;
}

void *typed_array::reserve(size_t len, long off)
{
    if (!_esize) {
        return 0;
    }
    // avoid incomplete element assignments
    size_t match = len / _esize;
    if (len != (match * _esize)) {
        return 0;
    }
    off *= _esize;
    // relative to data end
    if (off < 0) {
        off += _d.length();
        if (off < 0) {
            return 0;
        }
    }
    return mpt_array_slice(&_d, off, len);
    
}
size_t maxsize(Slice<const typed_array> sl, int type)
{
    const typed_array *arr = sl.base();
    size_t len = 0;
    for (size_t i = 0, max = sl.length(); i < max; ++i) {
        if (!arr->elementSize()) {
            continue;
        }
        if (type >= 0 && type != arr->type()) {
            continue;
        }
        size_t curr = arr->elements();
        if (curr > len) len = curr;
    }
    return len;
}

__MPT_NAMESPACE_END
