/*
 * MPT C++ library
 *   graphic instance operations
 */

#define __STDC_LIMIT_MACROS
#include <limits>
#include <cctype>

#include <sys/uio.h>

#include "message.h"

#include "layout.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

// update registration
graphic::update_hint::update_hint(int l, int g, int w) : match(0), lay(0), grf(0), wld(0)
{
    if (l >= 0 && l <= UINT8_MAX) { lay = l; match |= laydest::MatchLayout; }
    if (g >= 0 && g <= UINT8_MAX) { grf = g; match |= laydest::MatchGraph; }
    if (w >= 0 && w <= UINT8_MAX) { wld = w; match |= laydest::MatchWorld; }
}
// update reduction
bool graphic::update_hint::merge(const update_hint &with, int mask)
{
    // unconditional update
    if (!match) {
        return true;
    }
    if (!with.match) {
        match = 0;
        return true;
    }
    // need same layout
    if (lay != with.lay && !(mask & laydest::MatchLayout)) {
        return false;
    }
    // match layout
    if (match == laydest::MatchLayout) {
        return true;
    }
    if (with.match == laydest::MatchLayout) {
        match = laydest::MatchLayout;
        return true;
    }
    // need same graph
    if (grf != with.grf && !(mask & laydest::MatchGraph)) {
        return false;
    }
    // match graph
    if (match == (laydest::MatchLayout | laydest::MatchGraph)) {
        return true;
    }
    if (!(with.match & laydest::MatchWorld) || !(mask & laydest::MatchWorld)) {
        match = laydest::MatchLayout | laydest::MatchGraph;
        return true;
    }
    // need same world
    if (wld == with.wld) {
        return true;
    }
    return false;
}
// update target
bool graphic::update_hint::destination(laydest *dst)
{
    if (match != laydest::MatchPath) {
        return false;
    }
    if (dst) {
        dst->lay = lay;
        dst->grf = grf;
        dst->wld = wld;
        dst->dim = 0;
    }
    return true;
}
// graphic interface
graphic::graphic()
{ }

graphic::~graphic()
{ }

// layout registration
int graphic::add_layout(layout *lay, bool reuse)
{
    if (!lay) {
        return BadArgument;
    }
    // insert layout on vacant position
    if (reuse) {
        reference_wrapper<layout> *b, *e;
        b = _layouts.begin();
        e = _layouts.end();
        for (reference_wrapper<layout> *c = b; c < e; ++c) {
            if (c->reference()) {
                continue;
            }
            c->set_reference(lay);
            return c - b;
        }
    }
    // append layout
    long pos = _layouts.length();
    if (!_layouts.insert(pos, lay)) {
        return BadOperation;
    }
    return pos;
}
// layout removal
int graphic::remove_layout(const layout *lay)
{
    if (!lay) {
        return BadArgument;
    }
    reference_wrapper<layout> *r = _layouts.begin();
    for (long i = 0, max = _layouts.length(); i < max; ++i) {
        if (r[i].reference() != lay) {
            continue;
        }
        r[i].set_reference(0);
        return i;
    }
    return MissingData;
}
// number of layouts
long graphic::layout_count() const
{
    return _layouts.count();
}
// layout creation
layout *graphic::create_layout()
{
    return new layout;
}

static ssize_t next_part(const message &msg, size_t len)
{
    const char *end, *dest;

    if (!len) {
        return -1;
    }
    if (len <= msg.used) {
        if (!(end = (char *) memchr(dest = (const char *) msg.base, ':', len))) {
            return MissingData;
        }
        return end - dest;
    }
    else if ((end = (char *) memchr(dest = (const char *) msg.base, ':', msg.used))) {
        return end - dest;
    }
    else {
        struct iovec *cont = msg.cont;
        size_t clen = msg.clen, total = msg.used;

        while (clen--) {
            size_t curr = cont->iov_len;
            if (len < curr) {
                clen = 0;
                curr = len;
            }
            if ((end = (char *) memchr(dest = (const char *) cont->iov_base, ':', curr))) {
                return total + (end - dest);
            }
            total += curr;
            ++cont;
        }
        return MissingData;
    }
}
static Graph::data *graph_data(span<const item<Graph::data> > gd, int pos)
{
    const item<Graph::data> *it;
    if (!(it = gd.nth(pos))) {
        return 0;
    }
    return it->reference();
}

int graphic::target(laydest &old, message &msg, size_t len) const
{
    message tmp;
    laydest dst = old;
    ssize_t part;
    char *end, buf[128];
    int match = 0;

    if (!len && !(len = msg.length())) {
        return MissingData;
    }

    if ((part = next_part(msg, len)) < 0) {
        return MissingData;
    }

    /* get layout */
    if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    tmp = msg;
    mpt_message_read(&tmp, part + 1, buf);

    reference_wrapper<layout> *r;
    layout *lay = 0;
    if (part) {
        int l = strtol(buf, &end, 0);
        if (end > buf) {
            if (l <= 0 || l > UINT8_MAX) {
                return BadValue;
            }
            r = _layouts.get(l - 1);
            if ((lay = r ? r->reference() : 0)) {
                dst.lay = l;
                match |= dst.MatchLayout;
            }
        }
        else {
            long max = _layouts.length();
            r = _layouts.begin();
            if (max >= UINT8_MAX) {
                max = UINT8_MAX - 1;
            }
            for (long i = 0; i < max; ++i) {
                layout *l = r[i].reference();
                const char *id;
                if (l && (id = l->alias()) && part == (ssize_t) strlen(id) && !memcmp(id, buf, part)) {
                    dst.lay = i + 1;
                    match |= dst.MatchLayout;
                    lay = l;
                    break;
                }
            }
        }
    }
    else if (dst.lay) {
        r = _layouts.get(dst.lay - 1);
        lay = r ? r->reference() : 0;
    }
    if (!lay) {
        return BadValue;
    }

    /* get graph */
    if ((part = next_part(tmp, len -= part + 1)) < 0) {
        return part;
    }
    if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    mpt_message_read(&tmp, part + 1, buf);

    span<const class item<Graph> > graphs = lay->graphs();
    const class item<Graph> *gi;
    Graph *grf = 0;
    if (part) {
        int g = strtol(buf, &end, 0);
        if (end > buf) {
            if (g <= 0 || g > UINT8_MAX) {
                return BadValue;
            }
            gi = graphs.nth(g - 1);
            if (gi && (grf = gi->reference())) {
                dst.grf = g;
                match |= dst.MatchGraph;
            }
        }
        else {
            size_t max = graphs.size();
            if (max >= UINT8_MAX) {
                max = UINT8_MAX - 1;
            }
            for (size_t i = 0; i < max; ++i) {
                gi = graphs.nth(i);
                if (gi->equal(buf, part)) {
                    if ((grf = gi->reference())) {
                        dst.grf = i + 1;
                        match |= dst.MatchGraph;
                    }
                    break;
                }
            }
        }
    }
    else if (dst.grf) {
        if ((gi = graphs.nth(dst.grf - 1))) {
            grf = gi->reference();
        }
    }
    if (!grf) {
        return BadValue;
    }

    /* get world */
    uint8_t dim;
    if ((part = next_part(tmp, len -= part + 1)) < 0) {
        if (len >= sizeof(buf)) {
            return MissingBuffer;
        }
        mpt_message_read(&tmp, part = len, buf);
        buf[part] = 0;
    }
    else if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    span<const class item<Graph::data> > gd = grf->worlds();
    World *wld = 0;
    if (part) {
        int w = strtol(buf, &end, 0);
        if (end > buf) {
            if (w <= 0 || w > UINT8_MAX) {
                return BadValue;
            }
            const Graph::data *d = graph_data(gd, w - 1);
            if (d && (wld = d->world.reference())) {
                dst.wld = w;
                match = dst.MatchWorld;
            }
        }
        else {
            size_t max = gd.size();
            if (max >= UINT8_MAX) {
                max = UINT8_MAX - 1;
            }
            for (size_t i = 0; i < max; ++i) {
                const class item<Graph::data> *it = gd.nth(i);
                Graph::data *ptr;
                if (it && (ptr = it->reference()) && it->equal(buf, part)) {
                    if ((wld = ptr->world.reference())) {
                        dst.wld = i + 1;
                        match |= dst.MatchWorld;
                    }
                    break;
                }
            }
        }
    }
    else if (dst.wld) {
        const Graph::data *d;
        if ((d = graph_data(gd, dst.wld - 1))) {
            wld = d->world.reference();
        }
    }
    if (!wld) {
        return BadValue;
    }
    char post;

    mpt_message_read(&tmp, part + 1, buf);

    /* dimension target */
    if (!mpt_message_read(&tmp, 1, &dim)) {
        return match;
    }
    if (mpt_message_read(&tmp, 1, &post) && post && !isspace(post)) {
        return MissingData;
    }
    if (dim == 'x') {
        dim = 0;
    }
    else if (dim == 'y') {
        dim = 1;
    }
    else if (dim == 'z') {
        dim = 2;
    }
    else if (dim >= '0' && dim <= '9') {
        dim = dim - '0';
    }
    else {
        return BadValue;
    }
    dst.dim = dim;
    match |= dst.MatchDimension;
    msg = tmp;
    old = dst;

    return match;
}
metatype *graphic::item(message &msg, size_t len) const
{
    message tmp = msg;
    ssize_t part;
    char buf[128];
    bool term = false;

    if (!len) len = msg.length();

    /* get layout part */
    if ((part = next_part(tmp, len)) >= 0) {
        if (part >= (ssize_t) sizeof(buf)) {
            return 0;
        }
        mpt_message_read(&tmp, part+1, buf);
        len -= part + 1;
    }
    else if (len > sizeof(buf)) {
        return 0;
    }
    else {
        mpt_message_read(&tmp, part = len, buf);
        len = 0;
        term = true;
    }
    int type = typeinfo<group *>::id();
    
    reference_wrapper<layout> *r = _layouts.begin();
    for (size_t i = 0, max = _layouts.length(); i < max; ++i) {
        layout *l;
        if (!(l = r[i].reference())) {
            continue;
        }
        const char *id;
        // layout name mismatch
        if (!(id = l->alias())) {
            if (part) {
                continue;
            }
        }
        else if (part != (ssize_t) strlen(id) || memcmp(id, buf, part)) {
            continue;
        }
        if (type < 0) {
            continue;
        }
        group *g = l;
        metatype *m = l;

        while (!term) {
            // get next part
            if ((part = next_part(tmp, len)) >= 0) {
                if (part >= (ssize_t) sizeof(buf)) {
                    return 0;
                }
                mpt_message_read(&tmp, part+1, buf);
                len -= part + 1;
            }
            // temporary buffer insufficient
            else if (len > sizeof(buf)) {
                return 0;
            }
            // final part
            else {
                mpt_message_read(&tmp, part = len, buf);
                len = 0;
                term = false;
                type = 0;
            }
            // find element by type and name
            metatype *m;
            if (!(m = group_relation(*g, 0).find(type, buf, part))) {
                return 0;
            }
            // last name part
            if (term) {
                break;
            }
            // need group for next path element
            if (m->conv(type, &g) < 0
                || !g) {
                return 0;
            }
        }
        msg = tmp;
        return m;
    }
    return 0;
}

// collect references for update trigger
bool graphic::register_update(const reference *, update_hint)
{
    return true;
}
void graphic::dispatch_updates()
{ }

__MPT_NAMESPACE_END
