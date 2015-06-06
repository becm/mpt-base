/*
 * MPT C++ library
 *   graphic instance operations
 */

#define __STDC_LIMIT_MACROS
#include <limits>

#include <sys/uio.h>

#include "plot.h"
#include "message.h"

#include "output.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

// update registration
UpdateHint::UpdateHint(int l, int g, int w) : match(0), lay(0), grf(0), wld(0)
{
    if (l >= 0 && l <= UINT8_MAX) { lay = l; match |= MatchLayout; }
    if (g >= 0 && g <= UINT8_MAX) { grf = g; match |= MatchGraph; }
    if (w >= 0 && w <= UINT8_MAX) { wld = w; match |= MatchWorld; }
}

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
    if (lay != with.lay && !(mask & MatchLayout)) {
        return false;
    }
    // match layout
    if (match == MatchLayout) {
        return true;
    }
    if (with.match == MatchLayout) {
        match = MatchLayout;
        return true;
    }
    // need same graph
    if (grf != with.grf && !(mask & MatchGraph)) {
        return false;
    }
    // match graph
    if (match == (MatchLayout | MatchGraph)) {
        return true;
    }
    if (!(with.match & MatchWorld) || !(mask & MatchWorld)) {
        match = MatchLayout | MatchGraph;
        return true;
    }
    // need same world
    if (wld == with.wld) {
        return true;
    }
    return false;
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
        return -3;
    }
    size_t max = _layouts.size();

    // insert layout on vacant position
    if (reuse) {
        for (size_t i = 0; i < max; ++i) {
            if (_layouts.get(i)) continue;
            return _layouts.set(i, lay) ? i : -1;
        }
    }
    // append layout
    return _layouts.insert(max, lay) ? max : -1;
}
// layout removal
int Graphic::removeLayout(const Layout *lay)
{
    if (!lay) {
        return -1;
    }
    for (size_t i = 0, max = _layouts.size(); i < max; ++i) {
        if (_layouts.get(i) != lay) continue;
        _layouts.set(i, 0);
        return i;
    }
    return -2;
}
// number of layouts
int Graphic::layoutCount() const
{
    int lay = 0;
    for (size_t i = 0, max = _layouts.size(); i < max; ++i) {
        if (!_layouts.get(i)) continue;
        ++lay;
    }
    return lay;
}
// layout creation
Layout *Graphic::createLayout()
{
    Layout *lay = new Layout;

    if (addLayout(lay) < 0) {
        delete lay;
        return 0;
    }
    return lay;
}

static ssize_t nextPart(const message &msg, size_t len)
{
    const char *end, *dest;

    if (!len) {
        return -1;
    }
    if (len <= msg.used) {
        if (!(end = (char *) memchr(dest = (const char *) msg.base, ':', len))) {
            return -2;
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
        return -2;
    }
}

int Graphic::target(laydest &addr, message &msg, size_t len) const
{
    message tmp;
    laydest dst(_lastTarget.lay, _lastTarget.grf, _lastTarget.wld);
    Layout *lay = 0;
    Graph *grf = 0;
    World *wld = 0;
    ssize_t part;
    char *end, buf[128];
    int mask = 0;

    if (!len && !(len = msg.length())) {
        return -2;
    }

    if ((part = nextPart(msg, len)) < 0) {
        return part;
    }

    /* get layout */
    if (part >= (ssize_t) sizeof(buf)) {
        return -3;
    }
    tmp = msg;
    mpt_message_read(&tmp, part+1, buf);

    if (part) {
        int l = strtol(buf, &end, 0);
        if (end > buf) {
            if (l <= 0 || l > UINT8_MAX) {
                return -3;
            }
            lay = _layouts.get(l-1);
            dst.lay = l;
        }
        else {
            size_t max = _layouts.size();
            if (max > UINT8_MAX) {
                return max = UINT8_MAX;
            }
            for (size_t i = 0; i < max; ++i) {
                Layout *l = _layouts.get(i);
                const char *id;
                if (l && (id = l->alias()) && part == (ssize_t) strlen(id) && !memcmp(id, buf, part)) {
                    dst.lay = i+1;
                    lay = l;
                    break;
                }
            }
        }
        mask |= 1;
    }
    else if (dst.lay) {
        lay = _layouts.get(dst.lay-1);
    }
    if (!lay) {
        return -2;
    }

    /* get graph */
    if ((part = nextPart(tmp, len -= part+1)) < 0) {
        return part;
    }
    if (part >= (ssize_t) sizeof(buf)) {
        return -3;
    }
    mpt_message_read(&tmp, part+1, buf);

    if (part) {
        int g = strtol(buf, &end, 0);
        if (end > buf) {
            if (g <= 0 || g > UINT8_MAX) {
                return -3;
            }
            grf = lay->graph(g-1);
            dst.grf = g;
        }
        else {
            size_t max = lay->graphCount();
            if (max > UINT8_MAX) {
                max = UINT8_MAX;
            }
            for (size_t i = 0; i < max; ++i) {
                const Item<Graph> &g = lay->graph(i);
                if (g.equal(buf, part)) {
                    dst.grf = i+1;
                    grf = g;
                    break;
                }
            }
        }
        mask |= 2;
    }
    else if (dst.grf) {
        grf = lay->graph(dst.grf-1);
    }
    if (!grf) {
        return -2;
    }

    /* get world */
    if ((part = nextPart(tmp, len -= part+1)) < 0) {
        if (len >= sizeof(buf)) {
            return -3;
        }
        mpt_message_read(&tmp, part = len, buf);
        buf[part] = 0;
    }
    else if (part >= (ssize_t) sizeof(buf)) {
        return -3;
    }
    else {
        message after;
        char dim, post;

        mpt_message_read(&tmp, part+1, buf);

        /* dimension target */
        if (!mpt_message_read(&tmp, 1, &dim)) {
            return -1;
        }
        after = tmp;
        if (mpt_message_read(&after, 1, &post) && post && !isspace(post)) {
            return -1;
        }
        if (dim == 'x') {
            dst.dim = 0;
        }
        else if (dim == 'y') {
            dst.dim = 1;
        }
        else if (dim == 'z') {
            dst.dim = 2;
        }
        else if (dim >= '0' && dim <= '9') {
            dst.dim = dim - '0';
        }
        else {
            return -3;
        }
        mask |= 8;
    }
    if (part) {
        int w = strtol(buf, &end, 0);
        if (end > buf) {
            if (w <= 0 || w > UINT8_MAX) {
                return -3;
            }
            wld = grf->world(w-1);
            dst.wld = w;
        }
        else {
            size_t max = grf->worldCount();
            if (max > UINT8_MAX) {
                max = UINT8_MAX;
            }
            for (size_t i = 0; i < max; ++i) {
                const Item<World> &it = grf->world(i);
                if (it.equal(buf, part)) {
                    dst.wld = i+1;
                    wld = it;
                    break;
                }
            }
        }
        mask |= 4;
    }
    else if (dst.wld) {
        wld = grf->world(dst.wld-1);
    }
    if (!wld) {
        return -3;
    }
    msg = tmp;
    addr = dst;

    return mask;
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
    for (size_t i = 0, max = _layouts.size(); i < max; ++i) {
        Layout *l;
        if (!(l = _layouts.get(i))) {
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

        Group *g = l;
        metatype *m = l;

        while (!term) {
            /* get next part */
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
            // find item by name
            if (!(m = GroupRelation(*g, 0).find(0, buf, part))) {
                return 0;
            }
            // last name part
            if (term) {
                break;
            }
            // need group for further path search
            if (!(g = m->cast<Group>())) {
                return 0;
            }
        }
        msg = tmp;
        return m;
    }
    return 0;
}

// collect references for update trigger
bool Graphic::registerUpdate(const metatype *, const UpdateHint &)
{ return true; }
void Graphic::dispatchUpdates()
{ }

__MPT_NAMESPACE_END
