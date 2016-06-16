/*
 * MPT C++ cycle implementation
 */

#include <cerrno>
#include <limits>

#include "array.h"

#include "layout.h"

extern "C" mpt::cycle *mpt_cycle_create(void)
{
    return new mpt::Cycle();
}

__MPT_NAMESPACE_BEGIN

template class Reference<Cycle>;
template class RefArray<Cycle>;

// linepart set operation
bool linepart::setTrim(float val)
{
    int v = mpt_linepart_code(val);
    if (v < 0) return false;
    _trim = v;
    return true;
}
bool linepart::setCut(float val)
{
    int v = mpt_linepart_code(val);
    if (v < 0) return false;
    _cut = v;
    return true;
}

Cycle::Cycle(int dim) : _act(0), _flags(Cycle::AutoGrow)
{ _dim = (dim >= 0) ? dim : 2; }

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
Polyline *Cycle::append()
{
    Polyline *pl;
    pl = new Polyline(_dim);
    if (_part.insert(_part.length(), pl)) return pl;
    pl->unref();
    return 0;
}

Polyline *Cycle::current() const
{ return _part.get(_act); }

Polyline *Cycle::part(int i) const
{ return _part.get(i); }

Polyline *Cycle::advance()
{
    Polyline *pl;
    if (++_act > _part.length()) {
        if (!(_flags & AutoGrow)) {
            _act = 0;
        }
        else if ((pl = append())) {
            return pl;
        }
        --_act;
        return 0;
    }
    if ((pl = _part.get(_act))) return pl;
    if (!_part.length()) return 0;
    pl = new Polyline(_dim);
    if (_part.set(_act, pl)) return pl;
    pl->unref();
    return 0;
}
int Cycle::size() const
{
    return _part.length();
}

bool Cycle::setSize(int cyc)
{
    if (!cyc) {
        _flags |= AutoGrow;
        return true;
    }
    if (cyc < 0 || cyc > std::numeric_limits<decltype(_act)>::max()) return false;
    if (!_part.resize(cyc)) return false;
    _flags &= ~AutoGrow;
    return true;
}
bool Cycle::updateTransform(const Transform &tr, bool force)
{
    bool update = false;

    for (size_t i = 0, max = size(); i < max; ++i) {
        Polyline *p;
        if (!(p = _part.get(i)) || !(force || p->modified())) {
            continue;
        }
        p->transform(tr);
        update = true;
    }
    return update;
}

__MPT_NAMESPACE_END
