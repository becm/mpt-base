/*
 * MPT C++ cycle implementation
 */

#include <cerrno>
#include <limits>

#include <sys/uio.h>

#include "message.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

template class Reference<Cycle>;
template class RefArray<Cycle>;

template <> int typeinfo<Cycle *>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<Cycle::Stage>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<typed_array>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}

typed_array *rawdata_stage::values(int dim, int type)
{
    if (dim < 0) {
        errno = EINVAL;
        return 0;
    }
    typed_array *arr;
    if ((arr = _d.get(dim))) {
        if (type >= 0 && type != arr->type()) {
            errno = EINVAL;
            return 0;
        }
        return arr;
    }
    if (!(arr = mpt_stage_data(this, dim))) {
        return 0;
    }
    if (!arr->setType(type)) {
        errno = EINVAL;
        return 0;
    }
    return arr;
}

Cycle::Cycle() : _act(0), _maxDimensions(0), _flags(0)
{ }
Cycle::~Cycle()
{ }
void Cycle::unref()
{
    delete this;
}

int Cycle::modify(unsigned dim, int type, const void *src, size_t len, const valdest *vd)
{
    int nc;
    if (_maxDimensions && dim >= _maxDimensions) {
        return BadArgument;
    }
    if (!vd || !(nc = vd->cycle)) {
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
    typed_array *a = st->rawdata_stage::values(dim, type);
    if (!a) {
        return BadValue;
    }
    long off = vd ? vd->offset : 0;
    void *ptr;
    if (!(ptr = a->reserve(len, off))) {
        return BadOperation;
    }
    a->setModified();
    if (src) {
        memcpy(ptr, src, len);
    } else {
        memset(ptr, 0, len);
    }
    // invalidate view
    st->invalidate();
    return a->flags();
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
