/*
 * line item implementation
 */

#include <cstdlib>

#include <sys/uio.h>

#include "array.h"

#include "layout.h"

// basic type overrride
extern "C" mpt::polyline *mpt_pline_create()
{
    return new mpt::Polyline;
}

__MPT_NAMESPACE_BEGIN

linepart *linepart::join(const linepart &lp)
{ return mpt_linepart_join(this, &lp); }

float linepart::cut() const
{ return mpt_linepart_real(_cut); }
float linepart::trim() const
{ return mpt_linepart_real(_trim); }

const char *polyline::format() const
{ return 0; }


bool Polyline::Parts::create(const Transform &tr, polyline &part)
{
    int dim = tr.dimensions();

    set(0);

    linepart *base = 0;
    size_t old = 0;

    const char *fmt = part.format();
    ssize_t total = part.truncate();

    for (int i = 0; i < dim; i++) {
        size_t len = total;
        const double *val;

        if (!fmt[i]) break;
        if (!(val = (double *) part.raw(i, len)) || fmt[i] != 'd') continue;

        int cut = 0;
        size_t plen = len;

        if (base && old) {
            cut = base->cutRaw();
            if (base->usr < len) plen = base->usr;
        }
        size_t pos = 0, next = 0;

        while (len && plen) {
            linepart pt = tr.part(i, val, plen);

            // first part may be reduced
            if (cut) {
                int tc = pt.cutRaw();
                if (cut > tc) pt.setRaw(cut, pt.trimRaw());
                cut = 0;
            }
            val += pt.raw;
            len -= pt.raw;

            // existing parts
            if (!(plen -= pt.raw) && pos < old) {
                int trim = base[pos].trimRaw();
                int tt = pt.trimRaw();
                // correct end trim
                if (trim < tt) trim = tt;
                pt.setRaw(pt.cutRaw(), trim);
                // correct raw offset
                tt = base[pos].raw - base[pos].usr;
                pt.raw += tt;
                val += tt;
                len -= tt;
                // continue in next part
                if (++pos < old) {
                    plen = base[pos].usr;
                    cut = base[pos].cutRaw();
                }
            }
            // save current part data
            if (next && base[old + next].join(pt)) continue;
            append(sizeof(pt), &pt);
            base = (linepart *) array::base();
            ++next;
        }
        // set new data active
        if (old) {
            len = next*sizeof(linepart);
            memmove(base, base+old, len);
            set(len);
        }
        old = next;
    }
    return old ? true : false;
}
size_t Polyline::Parts::userLength()
{
    Slice<const linepart> d = data();
    return mpt_linepart_ulength(d.base(), d.length());
}
size_t Polyline::Parts::rawLength()
{
    Slice<const linepart> d = data();
    return mpt_linepart_rlength(d.base(), d.length());
}


Polyline::Polyline(int dim) : _rawSize(sizeof(double)), _dataSize(sizeof(dpoint))
{
    _rawType = (char* ) malloc(dim+1);

    for (int i = 0; i < dim; i++) _rawType[i] = 'd';
    _rawType[dim] = 0;

    if (dim < 0) dim = 2;
    _rawDim = dim;
    _d.insert(dim + 1, array());
}

Polyline::~Polyline()
{
    free(_rawType);
}

void *Polyline::raw(int dim, size_t need, size_t offset)
{
    array *a = rawData(dim);
    if (!a) return 0;
    size_t old = a->length();
    void *pos;
    if (!(pos = mpt_array_slice(a, offset*_rawSize, need*_rawSize))) {
        return 0;
    }
    if (old != a->length()) {
        setModified(dim);
    }
    return pos;
}

ssize_t Polyline::truncate(int dim, ssize_t pos)
{
    array *raw;

    if (dim < 0) {
        // set to max used size
        pos = mpt_pline_truncate(rawData(0), _rawDim, _rawSize * pos);
        if (pos < 0) {
            return pos;
        }
        setModified(dim);
        return pos / _rawSize;
    }
    if (!(raw = rawData(dim))) return -1;
    if (pos < 0) return raw->length() / _rawSize;

    if (!raw->set(pos * _rawSize)) {
        return -1;
    }
    setModified(dim);

    return pos;
}

void Polyline::setModified(int dim)
{
    if (dim < 0) {
        _modified = -1;
    } else {
        _modified |= 1<<dim;
    }
}

bool Polyline::modified(int dim) const
{
    if (dim < 0) {
        return _modified ? true : false;
    }
    return (_modified & (1<<dim)) ? true : false;
}

const char *Polyline::format() const
{ return _rawType; }

void Polyline::unref()
{
    delete this;
}

Slice<dpoint> Polyline::values(int part) const
{
    const array *arr;
    size_t len;
    Slice<const linepart> pt = vis();

    if (!(arr = userData()) || !(len = pt.length()) || part >= (int) len) {
        return Slice<dpoint>(0, 0);
    }

    dpoint *base = (dpoint *) arr->base();
    size_t used = arr->length() / sizeof(dpoint);

    dpoint *pos = base;
    for (int i = 0; i <= part; i++) {
        size_t adv = pt.base()->usr;
        base = pos;
        used = adv;
        pos += adv;
        pt.skip(1);
    }
    return Slice<dpoint>(base, used);
}

Slice<const linepart> Polyline::vis() const
{
    array *a;
    if (!(a = _d.get(0))) {
        return Slice<const linepart>(0, 0);
    }
    return static_cast<Parts *>(a)->data();
}

array *Polyline::rawData(int dim) const
{
    if (dim < 0 || dim >= _rawDim) return 0;
    return _d.get(2+dim);
}

Polyline::Points *Polyline::userData() const
{
    array *d = _d.get(1);
    return d ? static_cast<Points*>(d) : 0;
}

bool Polyline::setValues(int dim, size_t len, const double *val, int ld, size_t offset)
{
    void *dest;
    if (!(dest = raw(dim, len, offset))) return 0;
    switch (_rawType[dim]) {
    case 'd': mpt::copy(len, val, ld, (double*) dest, 1); break;
    case 'f': mpt::copy(len, val, ld, (float*)  dest, 1); break;
    default: return false;
    }
    setModified(dim);
    return true;
}

int applyLineData(dpoint *dest, const linepart *lp, int plen, const Transform &tr, polyline &part)
{
    int dim = tr.dimensions(), proc = 0;

    const char *fmt = part.format();
    ssize_t len = lp ? mpt_linepart_rlength(lp, plen) : part.truncate();

    for (int i = 0; i < dim; i++) {
        double *from;

        if (!fmt[i]) break;
        if (fmt[i] != 'd' || !(from = (double *) part.raw(i, len))) continue;
        ++proc;

        dpoint *to = dest;

        if (lp) {
            for (int j = 0; j < plen; j++) {
                tr.apply(i, lp[j], to, from);
                to   += lp[j].usr;
                from += lp[j].raw;
            }
        }
        else {
            linepart tmp;

            while (plen > std::numeric_limits<decltype(tmp.usr)>::max()) {
                tmp.usr = tmp.raw = std::numeric_limits<decltype(tmp.usr)>::max();
                plen -= tmp.usr;
                tr.apply(i, tmp, to, from);
                to   += tmp.usr;
                from += tmp.raw;
            }
            tmp.usr = tmp.raw = plen;
            tr.apply(i, tmp, to, from);
        }
    }
    return proc;
}

void Polyline::transform(const Transform &tr)
{
    // generate parts data
    Parts *a;
    if (!(a = static_cast<Parts *>(_d.get(0)))) return;
    a->create(tr, *this);

    // prepare target data
    size_t total = a->userLength();
    Points *val;
    if (!(val = userData())) return;
    dpoint *pts;
    if (!(pts = val->resize(total))) return;

    // set start values
    dpoint z = tr.zero();
    copy(total, &z, 0, pts, 1);

    // modify according to transformation
    Slice<const linepart> lp = a->data();
    applyLineData(pts, lp.base(), lp.length(), tr, *this);

    _modified = 0;
}

Slice<const dpoint> Polyline::Points::data() const
{
    return Slice<const dpoint>((dpoint *) base(), length() / sizeof(dpoint));
}
dpoint *Polyline::Points::resize(size_t len)
{
    return (dpoint *) set(len * sizeof(dpoint));
}

__MPT_NAMESPACE_END

