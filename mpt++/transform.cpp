/*
 * MPT transformation routines
 */

#include <math.h>
#include <float.h>

#include "array.h"

#include "layout.h"

__MPT_NAMESPACE_BEGIN

#ifndef _GNU_SOURCE
# define exp10(x)  exp(M_LN10 * (x))
#endif

range::range() : min(DBL_MIN), max(DBL_MAX)
{ }

value_apply::value_apply(int flg)
{
    mpt_value_apply_init(this, flg);
}
void value_apply::set(const struct range &r, int flg)
{
    double min, max;

    // log. scale
    if (flg & TransformLg) {
        if (flg & AxisLimitSwap) {
            min = floor(r.max);
            max = ceil(r.min);
        } else {
            min = floor(r.min);
            max = ceil(r.max);
        }
        if (min <= max) {
            max = min + 1;
        }
        min = exp10(min);
        max = exp10(max);
    }
    // normalize range
    else if (flg & AxisLimitSwap) {
        min = r.max;
        max = r.min;
    }
    else {
        min = r.min;
        max = r.max;
    }
    double fact = 1.0/(max - min);
    
    scale = fact;
    add   = min * fact;
}

// interface for transformation
void transform::unref()
{
    delete this;
}
int transform::dimensions() const
{
    return 0;
}
point<double> transform::zero() const
{
    return point<double>();
}
linepart transform::part(unsigned , const double *from, int len) const
{
     linepart p;
     mpt_linepart_linear(&p, from, len, 0);
     return p;
}
bool transform::apply(unsigned , const linepart &, point<double> *, const double *) const
{
    return false;
}

// implementation with 3 dimensions
Graph::transform::data::data(int flg) : value_apply(flg < 0 ? 0 : flg)
{
    if (flg < 0) {
        _flags = 0;
    } else {
        _flags = flg;
    }
}
Graph::transform::transform()
{
    new (&_dim[0]) data(AxisStyleX);
    new (&_dim[1]) data(AxisStyleY);
    new (&_dim[2]) data(AxisStyleZ);
}
Graph::transform::~transform()
{ }

int Graph::transform::dimensions() const
{
    if (_dim[2].to.x || _dim[2].to.y) {
        return 3;
    }
    if (_dim[1].to.x || _dim[1].to.y) {
        return 2;
    }
    if (_dim[0].to.x || _dim[0].to.y) {
        return 1;
    }
    return 0;
}
point<double> Graph::transform::zero() const
{
    return point<double>(_base.x, _base.y);
}
linepart Graph::transform::part(unsigned dim, const double *val, int len) const
{
    struct range l;
    const data *curr = 0;

    switch (dim) {
    case 0: curr = &_dim[0]; break;
    case 1: curr = &_dim[1]; break;
    case 2: curr = &_dim[2]; break;
    }
    linepart lp;
    
    if (!curr
     || !(curr->_flags & TransformLimit)) {
        mpt_linepart_linear(&lp, val, len, 0);
        return lp;
    }
    if (curr->_flags & TransformLg) {
        l.min = exp10(floor(curr->limit.min));
        l.max = exp10(ceil(curr->limit.max));
    } else {
        l = curr->limit;
    }
    mpt_linepart_linear(&lp, val, len, &l);

    if (curr->_flags & TransformLg) {
        double cut  = lp.cut();
        double trim = lp.trim();

        if (cut) cut = log10(cut);
        if (trim) trim = log10(trim);

        lp.set_cut(cut);
        lp.set_trim(trim);
    }
    return lp;
}

template void apply<point<double>, double>(point<double> *, const linepart &, const double *, const point<double> &);
template void apply<double>(point<double> *, const linepart &, const double *, const value_apply &);

extern void apply_log(point<double> *dest, const linepart &pt, const double *from, const value_apply &a)
{
    linepart all = pt;
    while (all.usr) {
        double tmp[32];
        linepart curr = all;
        if (curr.usr > 32) {
            curr.usr = 32;
            curr._trim = 0;
        }
        for (uint16_t i = 0; i < curr.usr; ++i) {
            tmp[i] = log10(from[i]);
        }
        ::mpt::apply<double>(dest, curr, tmp, a);
        from += curr.usr;
        dest += curr.usr;
        all.usr -= curr.usr;
        all.raw -= curr.usr;
        all._cut = 0;
    }
}

bool Graph::transform::apply(unsigned dim, const linepart &pt, point<double> *dest, const double *from) const
{
    const data *curr;

    if (!from) {
        return false;
    }
    // select transformation
    if (dim > 2) {
        return false;
    }
    size_t len;
    if (!(len = pt.usr)) {
        return true;
    }
    curr = &_dim[dim];
    // apply point data
    if (!(curr->_flags & TransformLg)) {
        ::mpt::apply<double>(dest, pt, from, *curr);
    }
    else {
        apply_log(dest, pt, from, *curr);
    }
    return true;
}

__MPT_NAMESPACE_END
