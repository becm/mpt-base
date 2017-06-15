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
