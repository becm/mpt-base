/*
 * MPT C++ cycle implementation
 */

#include <cerrno>
#include <limits>

#include <sys/uio.h>

#include "values.h"

__MPT_NAMESPACE_BEGIN

template class Reference<Cycle>;
template class RefArray<Cycle>;

int rawdata_stage::modify(unsigned int dim, int type, const void *src, size_t off, size_t len)
{
    typed_array *arr;
    int curr;

    if (!(arr = mpt_stage_data(this, dim))) {
        return BadValue;
    }
    if (!(curr = arr->type())) {
        if (!arr->setType(type)) {
            return BadType;
        }
    }
    else if (type != curr) {
        return BadType;
    }
    if (len) {
        void *dest;
        if (!(dest = arr->reserve(off, len))) {
            return BadOperation;
        }
        arr->setModified();
        if (src) memcpy(dest, src, len);
        else memset(dest, 0, len);
    }
    return arr->flags();
}

Cycle::Cycle() : _act(0), _maxDimensions(0), _flags(0)
{ }
Cycle::~Cycle()
{ }
void Cycle::unref()
{
    delete this;
}
uintptr_t Cycle::addref()
{
    return 0;
}

int Cycle::modify(unsigned dim, int type, const void *src, size_t off, size_t len, int nc)
{
    if (_maxDimensions && dim >= _maxDimensions) {
        return BadArgument;
    }
    if (nc < 0) {
        nc = _act;
    }
    Stage *st;
    if (!(st = _stages.get(nc))) {
        if (_flags & LimitStages) {
            return BadValue;
        }
        if (!_stages.insert(nc, Stage())
            || !(st = _stages.get(nc))) {
            return BadOperation;
        }
    }
    int code = st->modify(dim, type, src, off, len);
    if (code < 0) {
        return code;
    }
    // invalidate view
    st->invalidate();
    return code;
}
int Cycle::values(unsigned dim, struct iovec *vec, int nc) const
{
    if (_maxDimensions && dim >= _maxDimensions) {
        return BadArgument;
    }
    if (nc < 0) {
        nc = _act;
    }
    Stage *st;
    if (!(st = _stages.get(nc))) {
        return BadValue;
    }
    const typed_array *arr;
    if (!(arr = st->rawdata_stage::values(dim))) {
        return BadValue;
    }
    if (vec) {
        if (arr->type()) {
            vec->iov_base = (void *) arr->base();
            vec->iov_len  = arr->elements() * arr->elementSize();
        } else {
            vec->iov_base = 0;
            vec->iov_len  = 0;
        }
    }
    return arr->type() + (arr->flags() & ~0xff);
}
int Cycle::advance()
{
    int n = _act + 1;
    rawdata_stage *st;
    if ((st = _stages.get(n))) {
        _act = n;
        return n;
    }
    if (_flags & LimitStages) {
        _act = 0;
        return _stages.length() ? 0 : BadValue;
    }
    // no advance if current stage not assigned
    if (!(st = _stages.get(_act))) {
        return _act;
    }
    // no advance if current stage empty
    int dim = st->dimensions();
    if (!dim) {
        return _act;
    }
    _act = n;
    return n;
}
int Cycle::dimensions(int n) const
{
    if (n < 0) {
        n = _act;
    }
    Stage *st = _stages.get(n);
    return st ? st->dimensions() : BadArgument;
}
int Cycle::stages() const
{
    return _stages.length();
}
bool Cycle::limitStages(size_t max)
{
    if (!max) {
        _flags &= LimitStages;
        return true;
    }
    size_t curr = _stages.length();
    if (max < curr) {
        return false;
    }
    if (curr < max && !_stages.insert(max - 1, Stage())) {
        return false;
    }
    _flags |= LimitStages;
    return true;
}
void Cycle::limitDimensions(uint8_t md)
{
    _maxDimensions = md;
}

bool Cycle::Stage::transform(const Transform &tr)
{
    return _values.set(tr, rawdata_stage::values());
}
__MPT_NAMESPACE_END
