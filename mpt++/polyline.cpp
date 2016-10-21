/*
 * line item implementation
 */

#include <cstdlib>
#include <limits>

#include <sys/uio.h>

#include "array.h"

#include "layout.h"

__MPT_NAMESPACE_BEGIN

linepart *linepart::join(const linepart lp)
{ return mpt_linepart_join(this, lp); }

float linepart::cut() const
{ return mpt_linepart_real(_cut); }
float linepart::trim() const
{ return mpt_linepart_real(_trim); }

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

bool linepart::array::set(size_t len)
{
    if (!len) {
        clear();
        return true;
    }
    linepart *lp;
    size_t num, max = std::numeric_limits<__decltype(lp->usr)>::max();

    num = len / max;
    if (len > num * max) ++num;
    if (!(lp = static_cast<__decltype(lp)>(_d.set(num * sizeof(*lp))))) {
         return false;
    }
    for (size_t i = 0; i < num; ++i) {
        if (len < max) {
            lp[i].usr = lp[i].raw = len;
            break;
        }
        lp[i].usr = lp[i].raw = max;
        len -= max;
    }
    return true;
}
bool linepart::array::apply(const Transform &tr, int dim, Slice<const double> src)
{
    size_t len, oldlen = 0;
    linepart pt;

    if (dim < 0 || dim >= tr.dimensions()) {
        return false;
    }
    if (!(len = src.length())) {
        return false;
    }
    const double *val = src.begin();

    if (!(oldlen = length())) {
        size_t pos = 0;
        while (pos < len) {
            pt = tr.part(dim, val + pos, len - pos);
            _d.append(sizeof(pt), &pt);
            ++oldlen;
            pos += pt.raw;
        }
        return true;
    }
    size_t pos = 0, next = 0;
    linepart *base, old;

    base = begin();
    old = *base;

    // process vissible points only
    size_t vlen = len < old.usr ? len : old.usr;

    while (len && pos < oldlen) {
        pt = tr.part(dim, val, vlen);

        // minimize leading line
        if (old._cut > pt._cut) pt._cut = old._cut;
        // same visible range
        if ((vlen == pt.usr)) {
            // minimize trailing line
            if (old._trim > pt._trim) pt._trim = old._trim;
            if (++pos < oldlen) {
                old = base[pos];
                vlen = old.usr;
            } else {
                len = 0;
            }
            pt.raw = old.raw;
        }
        // more data in old part
        else if ((old.raw -= pt.raw)) {
            old._cut = 0;
            old.usr -= pt.raw;
            vlen = old.usr;
        }
        // continue in next part
        else if (++pos < oldlen) {
            old = base[pos];
            vlen = old.usr;
        }
        // continue in next part
        else {
            len = 0;
        }
        len -= pt.raw;
        val += pt.raw;

        // save current part data
        if (next && base[oldlen+next].join(pt)) continue;
        _d.append(sizeof(pt), &pt);
        base = begin();
        ++next;
    }
    // set new data active
    len = next * sizeof(*base);
    memmove(base, base+oldlen, len);
    _d.set(len);
    return true;
}
size_t linepart::array::userLength()
{
    size_t len = 0;
    for (auto p : *this) {
        len += p.usr;
    }
    return len;
}
size_t linepart::array::rawLength()
{
    size_t len = 0;
    for (auto p : *this) {
        len += p.raw;
    }
    return len;
}

int applyLineData(point<double> *dest, const linepart *lp, int plen, const Transform &tr, Slice<const typed_array> st)
{
    int dim, proc = 0;

    dim = tr.dimensions();

    for (int i = 0; i < dim; i++) {
        const typed_array *arr;

        if (!(arr = st.nth(i))) {
            break;
        }
        const double *from;
        int max;
        if (arr->type() != typeIdentifier(*from)) {
            continue;
        };
        from = static_cast<__decltype(from)>(arr->base());
        if ((max = arr->elements()) <= 0) {
            continue;
        }
        linepart tmp;
        point<double> *to = dest;

        if (lp) {
            for (int j = 0; j < plen; j++) {
                if (max < lp[j].usr) {
                     tmp = lp[j];
                     tmp.usr = max;
                     tr.apply(i, tmp, to, from);
                     break;
                }
                tr.apply(i, lp[j], to, from);
                to   += lp[j].usr;
                from += lp[j].raw;
            }
        }
        else {
	    if (max < plen) plen = max;
            while (plen > std::numeric_limits<__decltype(tmp.usr)>::max()) {
                tmp.usr = tmp.raw = std::numeric_limits<__decltype(tmp.usr)>::max();
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

bool Polyline::set(const Transform &tr, Slice<const typed_array> src)
{
    // generate parts data
    size_t max = maxsize(src, typeIdentifier<double>());
    if (!max || !_vis.set(max)) {
        return false;
    }
    const typed_array *arr = src.base();
    for (size_t i = 0, max = src.length(); i < max; ++i) {
        if (arr->type() != typeIdentifier<double>()) {
            continue;
        }
        _vis.apply(tr, i, Slice<const double>(static_cast<const double *>(arr->base()), arr->elements()));
    }

    // prepare target data
    size_t total = _vis.userLength();
    Point *pts;
    if (!(pts = _values.resize(total))) {
        _values.clear();
        return false;
    }
    // set start values
    point<double> z = tr.zero();
    copy(total, &z, 0, pts, 1);

    // modify according to transformation
    applyLineData(pts, _vis.begin(), _vis.length(), tr, src);

    return true;
}

// polyline iterator operations
Polyline::iterator Polyline::begin() const
{
    return Polyline::iterator(_vis.slice(), _values.begin());
}
Polyline::iterator Polyline::end() const
{
    return Polyline::iterator(Slice<const linepart>(_vis.end(), 0), _values.end());
}
Polyline::iterator &Polyline::iterator::operator++()
{
    if (!_parts.length()) {
        return *this;
    }
    const linepart *curr = _parts.base();
    _points += curr->usr;
    _parts.skip(1);
    return *this;
}

// polyline part operations
Polyline::Part Polyline::iterator::operator *() const
{
    if (!_parts.length()) {
        return Part(linepart(0), 0);
    }
    const linepart *curr = _parts.base();
    return Part(*curr, _points);
}
Slice<const Polyline::Point> Polyline::Part::points() const
{
    size_t len = _part.usr;
    const Point *pts = _pts;
    if (_part._cut) {
        ++pts;
        --len;
    }
    if (_part._trim) {
        --len;
    }
    return Slice<const Point>(pts, len);
}
Slice<const Polyline::Point> Polyline::Part::line() const
{
    return Slice<const Point>(_pts, _part.usr);
}

__MPT_NAMESPACE_END

