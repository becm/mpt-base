/*
 * MPT C++ cycle implementation
 */

#include <cerrno>
#include <limits>

#include <sys/uio.h>

#include "layout.h"

__MPT_NAMESPACE_BEGIN

template class Reference<Cycle>;
template class RefArray<Cycle>;

int cycle::modify(unsigned int dim, int type, const void *src, size_t off, size_t len)
{
    typed_array *arr;
    int curr;

    if (!(arr = mpt_cycle_data(this, dim, ValueCreate))) {
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
    return arr->type() + (arr->flags() & ~0xff);
}

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
    cycle *c;
    if (!(c = _cycles.get(nc))) {
        return BadValue;
    }
    return c->modify(dim, type, src, off, len);
}
int Cycle::values(unsigned dim, struct iovec *vec, int nc) const
{
    if (_maxDimensions && dim >= _maxDimensions) {
        return BadArgument;
    }
    if (nc < 0) {
        nc = _act;
    }
    cycle *c;
    if (!(c = _cycles.get(nc))) {
        return BadValue;
    }
    const typed_array *arr;
    if (!(arr = c->values(dim))) {
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
    int nc = _act + 1;
    cycle *c;
    if ((c = _cycles.get(nc))) {
        _act = nc;
        return nc;
    }
    nc = _cycles.length();
    if (!(_flags & LimitCycles)
        || !_cycles.insert(nc, cycle())) {
        return BadOperation;
    }
    return nc;
}
int Cycle::dimensions(int nc) const
{
    if (nc < 0) {
        nc = _act;
    }
    cycle *c = _cycles.get(nc);
    return c ? c->dimensions() : BadArgument;
}
int Cycle::cycles() const
{
    return _cycles.length();
}
bool Cycle::limitCycles(size_t max)
{
    if (!max) {
        _flags &= LimitCycles;
        return true;
    }
    size_t curr = _cycles.length();
    if (max < curr) {
        return false;
    }
    if (curr < max && !_cycles.insert(max - 1, cycle())) {
        return false;
    }
    _flags |= LimitCycles;
    return true;
}
void Cycle::limitDimensions(uint8_t md)
{
    _maxDimensions = md;
}
__MPT_NAMESPACE_END
