/*
 * MPT transformation routines
 */

#include <math.h>

#include "array.h"

#include "layout.h"

__MPT_NAMESPACE_BEGIN

#ifndef _GNU_SOURCE
# define exp10(x)  exp(M_LN10 * (x))
#endif

transform::transform(AxisFlag flg)
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
        this->min = max;
        this->max = min;
    } else {
        this->min = min;
        this->max = max;
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

dpoint Transform::zero() const
{ return dpoint(); }

linepart Transform::part(int , const double *from, int len) const
{ linepart p; mpt_linepart_linear(&p, from, len, 0); return p; }
bool Transform::apply(int , const linepart &, dpoint *, const double *) const
{ return false; }

// implementation with 3 dimensions
Transform3::Transform3() : tx(AxisStyleX), ty(AxisStyleY), tz(AxisStyleZ), fx(0), fy(0), fz(0), cut(0)
{ }

int Transform3::dimensions() const
{ return (tz.scale.x || tz.scale.y) ? 3 : 2; }

dpoint Transform3::zero() const
{
    double x = tx.move.x + ty.move.x + tz.move.x;
    double y = tx.move.y + ty.move.y + tz.move.y;

    return dpoint(x,y);
}
linepart Transform3::part(int dim, const double *val, int len) const
{
    dpoint range;
    bool log = false;

    switch (dim) {
    case 0: range.x = tx.min; range.y = tx.max; if (fx & AxisLg) log = true; break;
    case 1: range.x = tx.min; range.y = tx.max; if (fy & AxisLg) log = true; break;
    case 2: range.x = tx.min; range.y = tx.max; if (fz & AxisLg) log = true; break;
    }
    linepart lp;
    if (log) {
        range.x = exp10(floor(range.x));
        range.y = exp10(ceil(range.y));
    }
    mpt_linepart_linear(&lp, val, len, (cut & (1<<dim)) ? &range : 0);

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


template void apply<dpoint, double>(dpoint *, const linepart &, const double *, const dpoint &, double (*)(double));

bool Transform3::apply(int dim, const linepart &pt, dpoint *dest, const double *from) const
{
    const dpoint *scale = 0;
    bool log = false;
    double zx = 0, zy;

    // select transformation
    switch (dim) {
    case 0: scale = &tx.scale; zx = tx.min; if (fx & AxisLg) log = true; break;
    case 1: scale = &ty.scale; zx = ty.min; if (fy & AxisLg) log = true; break;
    case 2: scale = &tz.scale; zx = tz.min; if (fz & AxisLg) log = true; break;
    }
    if (!from || !scale) return false;

    size_t len;

    if (!(len = pt.usr)) return true;

    // apply point data
    if (!log) {
        //::mpt::apply<dpoint, double>(dest, pt, from, *scale, retSelf);
        mpt_apply_linear(dest, &pt, from, scale);
    }
    else {
        ::mpt::apply<dpoint, double>(dest, pt, from, *scale, log10);
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
