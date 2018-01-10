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

transform::transform(AxisFlags flg)
{ mpt_trans_init(this, flg); }

int transform::fromAxis(const axis &a, int type)
{
    double min = a.begin, max = a.end;

    if (type < 0) {
        type = a.format & AxisStyles;
    }
    // log. scale
    if (a.format & AxisLg) {
        if (min > max) {
            type |= TransformSwap;
            min = floor(a.end);
            max = ceil(a.begin);
        } else {
            min = floor(min);
            max = ceil(max);
        }
        if (min <= max) {
            max = min + 1;
        }
        min = exp10(min);
        max = exp10(max);
        type |= AxisLg;
    }
    // normalize range
    else if (min > max) {
        type |= TransformSwap;
        this->limit.min = max;
        this->limit.max = min;
    } else {
        this->limit.min = min;
        this->limit.max = max;
    }
    double fact = 1.0/(max - min);

    switch (type) {
    case AxisStyleX:
        scale.x = fact;
        scale.y = 0;
        break;
    case AxisStyleY:
        scale.x = 0;
        scale.y = fact;
        break;
    case AxisStyleZ:
        scale.x = fact * M_SQRT1_2;
        scale.y = fact * M_SQRT1_2;
        break;
    default:
        scale.x = scale.y = 0;
    }
    move.x = -min * scale.x;
    move.y = -min * scale.y;

    return type;
}

int Transform::dimensions() const
{ return 0; }

point<double> Transform::zero() const
{ return point<double>(); }

linepart Transform::part(unsigned , const double *from, int len) const
{
     linepart p;
     mpt_linepart_linear(&p, from, len, 0);
     return p;
}
bool Transform::apply(unsigned , const linepart &, point<double> *, const double *) const
{ return false; }

// implementation with 3 dimensions
Transform3::Transform3() : tx(AxisStyleX), ty(AxisStyleY), tz(AxisStyleZ), fx(0), fy(0), fz(0), cutoff(0xff)
{ }

void Transform3::unref()
{
    delete this;
}

int Transform3::dimensions() const
{
    return (tz.scale.x || tz.scale.y) ? 3 : 2;
}
point<double> Transform3::zero() const
{
    double x = tx.move.x + ty.move.x + tz.move.x;
    double y = tx.move.y + ty.move.y + tz.move.y;

    return point<double>(x,y);
}
linepart Transform3::part(unsigned dim, const double *val, int len) const
{
    struct range l;
    bool log = false;

    switch (dim) {
    case 0: l = tx.limit; if (fx & AxisLg) log = true; break;
    case 1: l = ty.limit; if (fy & AxisLg) log = true; break;
    case 2: l = tz.limit; if (fz & AxisLg) log = true; break;
    }
    linepart lp;
    if (log) {
        l.min = exp10(floor(l.min));
        l.max = exp10(ceil(l.max));
    }
    mpt_linepart_linear(&lp, val, len, (cutoff & (1<<dim)) ? &l : 0);

    if (log) {
        double cut  = lp.cut();
        double trim = lp.trim();

        if (cut) cut = log10(cut);
        if (trim) trim = log10(trim);

        lp.setCut(cut);
        lp.setTrim(trim);
    }
    return lp;
}

template void apply<point<double>, double>(point<double> *, const linepart &, const double *, const point<double> &);

extern void apply_log(point<double> *dest, const linepart &pt, const double *from, const point<double> &fact)
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
        ::mpt::apply<double>(dest, curr, tmp, fact);
        from += curr.usr;
        dest += curr.usr;
        all.usr -= curr.usr;
        all.raw -= curr.usr;
        all._cut = 0;
    }
}

bool Transform3::apply(unsigned dim, const linepart &pt, point<double> *dest, const double *from) const
{
    const dpoint *scale;
    bool log = false;
    double zx = 0, zy;

    if (!from) {
        return false;
    }
    // select transformation
    switch (dim) {
    case 0: scale = &tx.scale; zx = tx.limit.min; if (fx & AxisLg) log = true; break;
    case 1: scale = &ty.scale; zx = ty.limit.min; if (fy & AxisLg) log = true; break;
    case 2: scale = &tz.scale; zx = tz.limit.min; if (fz & AxisLg) log = true; break;
    default: return false;
    }
    size_t len;
    if (!(len = pt.usr)) return true;

    // apply point data
    point<double> fact(scale->x, scale->y);
    if (!log) {
        ::mpt::apply<double>(dest, pt, from, fact);
    }
    else {
        apply_log(dest, pt, from, fact);
        if (zx) zx = log10(zx);
    }
    // remove difference to axis start
    if (!zx) return true;

    zy = scale->y * zx;
    zx = scale->x * zx;

    for (int i = 0; i < pt.usr; i++) {
        dest[i].x -= zx;
        dest[i].y -= zy;
    }
    return true;
}

__MPT_NAMESPACE_END
