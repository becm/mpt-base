/*
 * MPT C++ typed array
 */

#include "message.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

// typed information for array
bool value_store::set_type(int t)
{
    if (t == static_cast<int>(_type)) {
        return true;
    }
    int esize;
    if ((esize = mpt_valsize(t)) <= 0 || esize > 0xff) {
        return false;
    }
    _type = t;
    _esize = esize;
    t = mpt_msgvalfmt_code(t);
    _code = t < 0 ? 0 : t;
    return true;
}
void value_store::set_modified(bool set)
{
    if (set) _flags |= ValueChange;
    else _flags &= ~ValueChange;
}

void *value_store::reserve(long len, long off)
{
    if (!_esize) {
        return 0;
    }
    // avoid incomplete element assignments
    size_t need = len * _esize;
    ssize_t pos = off * _esize;
    // relative to data end
    if (pos < 0) {
        pos += _d.length();
        if (pos < 0) {
            return 0;
        }
    }
    return mpt_array_slice(&_d, pos, need);
}
size_t maxsize(Slice<const value_store> sl, int type)
{
    const value_store *val = sl.base();
    size_t len = 0;
    for (size_t i = 0, max = sl.length(); i < max; ++i) {
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
