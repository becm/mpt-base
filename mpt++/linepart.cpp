/*
 * MPT C++ library
 *   line part operations
 */

#include <cstdlib>
#include <limits>

#include <sys/uio.h>

#include "values.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<linepart>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}

// partial line value
linepart *linepart::join(const linepart lp)
{
    return mpt_linepart_join(this, lp);
}
float linepart::cut() const
{
    return mpt_linepart_real(_cut);
}
float linepart::trim() const
{
    return mpt_linepart_real(_trim);
}
bool linepart::set_trim(float val)
{
    int v = mpt_linepart_code(val);
    if (v < 0) return false;
    _trim = v;
    return true;
}
bool linepart::set_cut(float val)
{
    int v = mpt_linepart_code(val);
    if (v < 0) return false;
    _cut = v;
    return true;
}
// initial part setup
bool linepart::array::set(long len)
{
    if (!len) {
        resize(0);
        return true;
    }
    linepart *lp;
    long num, max = std::numeric_limits<__decltype(lp->usr)>::max() - 2;
    // determine existing points
    if (len < 0) {
        num = length();
        len = 0;
        lp = begin();
        for (long i = 0; i < num; ++i) {
            len += lp[i].raw;
        }
    }
    // reserve total size
    num = len / max;
    if (len > num * max) ++num;
    if (!resize(num)) {
        return false;
    }
    lp = begin();
    for (long i = 0; i < num; ++i) {
        if (len < max) {
            lp[i].usr = lp[i].raw = len;
            resize(i + 1);
            return true;
        }
        lp[i].usr = lp[i].raw = max;
        len -= max;
    }
    return true;
}
// modify parts to include dimension data
bool linepart::array::apply(const Transform &tr, int dim, Slice<const double> src)
{
    if (dim < 0 || dim >= tr.dimensions()) {
        return false;
    }
    long len;
    if (!(len = src.length())) {
        return false;
    }
    const double *val = src.begin();

    long oldlen;
    linepart pt;
    if (!(oldlen = length())) {
        long pos = 0;
        while (pos < len) {
            pt = tr.part(dim, val + pos, len - pos);
            insert(oldlen++, pt);
            pos += pt.raw;
        }
        return true;
    }
    long pos = 0, next = 0;
    linepart *base = begin(), old = *base;
    
    while (pos < oldlen) {
        // no visible points
        if (!old.usr || !len) {
            pt = old;
            // skip invisible data
            if (len > pt.raw) {
                len -= pt.raw;
                val += pt.raw;
            }
            // no further visible data
            else {
                len = 0;
            }
            // continue in next part
            if (++pos < oldlen) {
                old = base[pos];
            }
        } else {
            if (len < old.usr) {
                old.usr = len;
            }
            pt = tr.part(dim, val, old.usr);
            // minimize leading line
            if (old._cut > pt._cut) {
                pt._cut = old._cut;
            }
            // partial segment
            if (pt.raw < old.raw) {
                old.raw -= pt.raw;
                old.usr -= pt.raw;
                old._cut = 0;
            }
            else {
                // smaller old segment
                if (old.raw < pt.raw) {
                    pt.raw = old.raw;
                }
                // minimize trailing line
                if (old._trim > pt._trim) {
                    pt._trim = old._trim;
                }
                // continue in next part
                if (++pos < oldlen) {
                    old = base[pos];
                }
            }
            len -= pt.raw;
            val += pt.raw;
        }
        // merge part data
        if (next && base[oldlen + next - 1].join(pt)) {
            continue;
        }
        // append current part data
        insert(oldlen + next, pt);
        base = begin();
        ++next;
    }
    // set new data active
    memmove(base, base + oldlen, next * sizeof(*base));
    resize(next);
    return true;
}
// query total part lenght
long linepart::array::length_user()
{
    long len = 0;
    for (auto p : *this) {
        len += p.usr;
    }
    return len;
}
long linepart::array::length_raw()
{
    long len = 0;
    for (auto p : *this) {
        len += p.raw;
    }
    return len;
}

__MPT_NAMESPACE_END

