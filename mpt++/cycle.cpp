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
template <> int typeinfo<value_store>::id()
{
    return mpt_value_store_typeid();
}

value_store *rawdata_stage::values(long dim)
{
    if (dim < 0) {
        dim += values().length();
        if (dim < 0) {
            errno = EINVAL;
            return 0;
        }
    }
    return mpt_stage_data(this, dim);
}

Cycle::Cycle() : _act(0), _max_dimensions(0), _flags(0)
{ }
Cycle::~Cycle()
{ }
void Cycle::unref()
{
    delete this;
}

int Cycle::modify(unsigned dim, int type, const void *src, size_t len, const valdest *vd)
{
    if (_max_dimensions && dim >= _max_dimensions) {
        return BadArgument;
    }
    int esize;
    if ((esize = mpt_valsize(type)) <= 0) {
        return BadType;
    }
    if (len % esize) {
        return MissingData;
    }
    len /= esize;
    int nc;
    if (!vd || !(nc = vd->cycle)) {
        nc = _act;
    } else {
        --nc;
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
    value_store *val = st->rawdata_stage::values(dim);
    if (!val) {
        return BadValue;
    }
    long off = vd ? vd->offset : 0;
    double *ptr;
    if (!(ptr = static_cast<double *>(val->reserve(typeinfo<double>::id(), len * sizeof(double), off)))) {
        return BadOperation;
    }
    if (!src) {
        memset(ptr, 0, len * sizeof(double));
    }
    else switch (type) {
        case typeinfo<long double>::id():
            copy(len, static_cast<const long double *>(src), ptr);
            break;
        case typeinfo<double>::id():
            memcpy(ptr, src, len * sizeof(double));
            break;
        case typeinfo<float>::id():
            copy(len, static_cast<const float *>(src), ptr);
            break;
        
        case typeinfo<int8_t>::id():
            copy(len, static_cast<const int8_t  *>(src), ptr);
            break;
        case typeinfo<int16_t>::id():
            copy(len, static_cast<const int16_t *>(src), ptr);
            break;
        case typeinfo<int32_t>::id():
            copy(len, static_cast<const int32_t *>(src), ptr);
            break;
        case typeinfo<int64_t>::id():
            copy(len, static_cast<const int64_t *>(src), ptr);
            break;
	    
        case typeinfo<uint8_t>::id():
            copy(len, static_cast<const uint8_t  *>(src), ptr);
            break;
        case typeinfo<uint16_t>::id():
            copy(len, static_cast<const uint16_t *>(src), ptr);
            break;
        case typeinfo<uint32_t>::id():
            copy(len, static_cast<const uint32_t *>(src), ptr);
            break;
        case typeinfo<uint64_t>::id():
            copy(len, static_cast<const uint64_t *>(src), ptr);
            break;
        default:
            return BadType;
    }
    // update dimension and invalidate view
    val->set_modified();
    st->invalidate();
    return val->flags();
}
int Cycle::values(unsigned dim, struct iovec *vec, int nc) const
{
    if (_max_dimensions && dim >= _max_dimensions) {
        return BadArgument;
    }
    if (nc < 0) {
        nc = _act;
    }
    Stage *st;
    if (!(st = _stages.get(nc))) {
        return BadValue;
    }
    const value_store *val;
    if (!(val = st->rawdata_stage::values(dim))) {
        return BadValue;
    }
    if (vec) {
        if (val->type()) {
            vec->iov_base = (void *) val->base();
            vec->iov_len  = val->element_count() * val->element_size();
        } else {
            vec->iov_base = 0;
            vec->iov_len  = 0;
        }
    }
    return val->type() + (val->flags() & ~0xff);
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
        return BadArgument;
    }
    // no advance if current stage empty
    long dim = st->dimension_count();
    if (dim <= 0) {
        return _act;
    }
    _act = n;
    return n;
}
long Cycle::dimension_count(int n) const
{
    if (n < 0) {
        n = _act;
    }
    Stage *st = _stages.get(n);
    return st ? st->dimension_count() : (long) BadArgument;
}
long Cycle::stage_count() const
{
    return _stages.length();
}
bool Cycle::limit_stages(size_t max)
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
void Cycle::limit_dimensions(uint8_t md)
{
    _max_dimensions = md;
}

bool Cycle::Stage::transform(const Transform &tr)
{
    return _values.set(tr, rawdata_stage::values());
}
__MPT_NAMESPACE_END
