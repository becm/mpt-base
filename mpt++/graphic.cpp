/*
 * MPT C++ library
 *   graphic instance operations
 */

#define __STDC_LIMIT_MACROS
#include <limits>
#include <cctype>

#include <sys/uio.h>

#include "message.h"
#include "output.h"

#include "layout.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

// update registration
UpdateHint::UpdateHint(int l, int g, int w) : match(0), lay(0), grf(0), wld(0)
{
    if (l >= 0 && l <= UINT8_MAX) { lay = l; match |= msgdest::MatchLayout; }
    if (g >= 0 && g <= UINT8_MAX) { grf = g; match |= msgdest::MatchGraph; }
    if (w >= 0 && w <= UINT8_MAX) { wld = w; match |= msgdest::MatchWorld; }
}
// update reduction
bool UpdateHint::merge(const UpdateHint &with, int mask)
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
    if (lay != with.lay && !(mask & msgdest::MatchLayout)) {
        return false;
    }
    // match layout
    if (match == msgdest::MatchLayout) {
        return true;
    }
    if (with.match == msgdest::MatchLayout) {
        match = msgdest::MatchLayout;
        return true;
    }
    // need same graph
    if (grf != with.grf && !(mask & msgdest::MatchGraph)) {
        return false;
    }
    // match graph
    if (match == (msgdest::MatchLayout | msgdest::MatchGraph)) {
        return true;
    }
    if (!(with.match & msgdest::MatchWorld) || !(mask & msgdest::MatchWorld)) {
        match = msgdest::MatchLayout | msgdest::MatchGraph;
        return true;
    }
    // need same world
    if (wld == with.wld) {
        return true;
    }
    return false;
}
// update target
bool UpdateHint::destination(msgdest *dst)
{
    if (match != msgdest::MatchPath) {
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
Graphic::Graphic()
{ }

Graphic::~Graphic()
{ }

// layout registration
int Graphic::addLayout(Layout *lay, bool reuse)
{
    if (!lay) {
        return BadArgument;
    }
    // insert layout on vacant position
    if (reuse) {
        Reference<Layout> *b, *e;
        b = _layouts.begin();
        e = _layouts.end();
        for (Reference<Layout> *c = b; c < e; ++c) {
            if (c->pointer()) continue;
            c->setPointer(lay);
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
int Graphic::removeLayout(const Layout *lay)
{
    if (!lay) {
        return -1;
    }
    Reference<Layout> *r = _layouts.begin();
    for (size_t i = 0, max = _layouts.length(); i < max; ++i) {
        if (r[i].pointer() != lay) {
            continue;
        }
        r[i].setPointer(0);
        return i;
    }
    return MissingData;
}
// number of layouts
long Graphic::layoutCount() const
{
    return _layouts.count();
}
// layout creation
Layout *Graphic::createLayout()
{
    return new Layout;
}

static ssize_t nextPart(const message &msg, size_t len)
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
static Graph::Data *graphData(Slice<const Item<Graph::Data> > gd, int pos)
{
    const Item<Graph::Data> *it = gd.nth(pos);
    if (!it) return 0;
    return it->pointer();
}

int Graphic::target(msgdest &old, message &msg, size_t len) const
{
    message tmp;
    msgdest dst = old;
    ssize_t part;
    char *end, buf[128];
    int match = 0;

    if (!len && !(len = msg.length())) {
        return MissingData;
    }

    if ((part = nextPart(msg, len)) < 0) {
        return MissingData;
    }

    /* get layout */
    if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    tmp = msg;
    mpt_message_read(&tmp, part + 1, buf);

    Reference<Layout> *r;
    Layout *lay = 0;
    if (part) {
        int l = strtol(buf, &end, 0);
        if (end > buf) {
            if (l <= 0 || l > UINT8_MAX) {
                return BadValue;
            }
            r = _layouts.get(l - 1);
            if ((lay = r ? r->pointer() : 0)) {
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
                Layout *l = r[i].pointer();
                const char *id;
                if (l && (id = l->alias()) && part == (ssize_t) strlen(id) && !memcmp(id, buf, part)) {
                    dst.lay = i + 1;
                    match |= msgdest::MatchLayout;
                    lay = l;
                    break;
                }
            }
        }
    }
    else if (dst.lay) {
        r = _layouts.get(dst.lay - 1);
        lay = r ? r->pointer() : 0;
    }
    if (!lay) {
        return BadValue;
    }

    /* get graph */
    if ((part = nextPart(tmp, len -= part + 1)) < 0) {
        return part;
    }
    if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    mpt_message_read(&tmp, part + 1, buf);

    Slice<const Item<Graph> > graphs = lay->graphs();
    const Item<Graph> *gi;
    Graph *grf = 0;
    if (part) {
        int g = strtol(buf, &end, 0);
        if (end > buf) {
            if (g <= 0 || g > UINT8_MAX) {
                return BadValue;
            }
            gi = graphs.nth(g - 1);
            if (gi && (grf = gi->pointer())) {
                dst.grf = g;
                match |= dst.MatchGraph;
            }
        }
        else {
            size_t max = graphs.length();
            if (max >= UINT8_MAX) {
                max = UINT8_MAX - 1;
            }
            for (size_t i = 0; i < max; ++i) {
                gi = graphs.nth(i);
                if (gi->equal(buf, part)) {
                    if ((grf = gi->pointer())) {
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
            grf = gi->pointer();
        }
    }
    if (!grf) {
        return BadValue;
    }

    /* get world */
    uint8_t dim;
    if ((part = nextPart(tmp, len -= part + 1)) < 0) {
        if (len >= sizeof(buf)) {
            return MissingBuffer;
        }
        mpt_message_read(&tmp, part = len, buf);
        buf[part] = 0;
    }
    else if (part >= (ssize_t) sizeof(buf)) {
        return MissingBuffer;
    }
    Slice<const Item<Graph::Data> > gd = grf->worlds();
    World *wld = 0;
    if (part) {
        int w = strtol(buf, &end, 0);
        if (end > buf) {
            if (w <= 0 || w > UINT8_MAX) {
                return BadValue;
            }
            const Graph::Data *d = graphData(gd, w - 1);
            if (d && (wld = d->world.pointer())) {
                dst.wld = w;
                match = dst.MatchWorld;
            }
        }
        else {
            size_t max = gd.length();
            if (max >= UINT8_MAX) {
                max = UINT8_MAX - 1;
            }
            for (size_t i = 0; i < max; ++i) {
                const Item<Graph::Data> *it = gd.nth(i);
                Graph::Data *ptr;
                if (it && (ptr = it->pointer()) && it->equal(buf, part)) {
                    if ((wld = ptr->world.pointer())) {
                        dst.wld = i + 1;
                        match |= dst.MatchWorld;
                    }
                    break;
                }
            }
        }
    }
    else if (dst.wld) {
        const Graph::Data *d;
        if ((d = graphData(gd, dst.wld - 1))) {
            wld = d->world.pointer();
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
metatype *Graphic::item(message &msg, size_t len) const
{
    message tmp = msg;
    ssize_t part;
    char buf[128];
    bool term = false;

    if (!len) len = msg.length();

    /* get layout part */
    if ((part = nextPart(tmp, len)) >= 0) {
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
    int type = Group::typeIdentifier();
    
    Reference<Layout> *r = _layouts.begin();
    for (size_t i = 0, max = _layouts.length(); i < max; ++i) {
        Layout *l;
        if (!(l = r[i].pointer())) {
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
        Group *g = l;
        metatype *m = l;

        while (!term) {
            // get next part
            if ((part = nextPart(tmp, len)) >= 0) {
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
            if (!(m = GroupRelation(*g, 0).find(type, buf, part))) {
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
bool Graphic::registerUpdate(const reference *, UpdateHint)
{ return true; }
void Graphic::dispatchUpdates()
{ }

__MPT_NAMESPACE_END
