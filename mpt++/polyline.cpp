/*
 * MPT C++ library
 *   polyline operations
 */

#include <cstdlib>
#include <limits>

#include <sys/uio.h>

#include "values.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<point<double> >::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}

int apply_data(point<double> *dest, const linepart *lp, int plen, const Transform &tr, Slice<const value_store> st)
{
    int dim, proc = 0;

    dim = tr.dimensions();

    for (int i = 0; i < dim; i++) {
        const value_store *val;

        if (!(val = st.nth(i))) {
            continue;
        }
        const double *from = 0;
        long max;
        if (val->type() != typeinfo<double>::id()) {
            continue;
        };
        from = static_cast<__decltype(from)>(val->base());
        if ((max = val->element_count()) <= 0) {
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
            if (max < plen) {
                plen = max;
            }
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
        ++proc;
    }
    return proc;
}

bool Polyline::set(const Transform &tr, Slice<const value_store> src)
{
    // generate parts data
    long max = maxsize(src, typeinfo<double>::id());
    if (!max || !_vis.set(max)) {
        return false;
    }
    const value_store *val = src.base();
    for (long i = 0, max = src.length(); i < max; ++i) {
        if (val->type() != typeinfo<double>::id()) {
            continue;
        }
        _vis.apply(tr, i, Slice<const double>(static_cast<const double *>(val->base()), val->element_count()));
        ++val;
    }
    // prepare target data
    if (!(max = _vis.length_user())) {
        _values.resize(0);
        return false;
    }
    if (!_values.resize(max)) {
        _values.resize(0);
        return false;
    }
    // set start values
    Point *pts = _values.begin();
    point<double> z = tr.zero();
    copy(max, &z, 0, pts, 1);

    // modify according to transformation
    apply_data(pts, _vis.begin(), _vis.length(), tr, src);

    return true;
}

// polyline iterator operations
Polyline::iterator Polyline::begin() const
{
    return Polyline::iterator(_vis.elements(), _values.begin());
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

