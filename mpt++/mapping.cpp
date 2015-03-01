/*
 * MPT C++ library
 *   cycle target mapping
 */


#include <limits>

#include "layout.h"
#include "message.h"

#include "output.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

// add data mapping
int Mapping::add(const msgbind &src, const laydest &dst, int client)
{
    if (!getCycle(dst)) {
        return -2;
    }
    mapping map(src, dst, client);
    return mpt_mapping_add(&_bind, &map);
}
// clear data mapping
int Mapping::del(const msgbind *src, const laydest *dest, int client) const
{
    return mpt_mapping_del(&_bind, src, dest, client);
}
// get data mapping
const mapping *Mapping::get(const msgbind &src, int client) const
{
    return mpt_mapping_get(&_bind, &src, client);
}
// clear data mappings
void Mapping::clear()
{
    mpt_array_clone(&_bind, 0);
    _d = Array<Element>();
}

// save cycles references
bool Mapping::saveCycles(int layId, int graphID, const Graph &graph)
{
    if (layId >= UINT16_MAX || graphID >= UINT16_MAX) {
        return false;
    }
    for (size_t i = 0, max = graph.worldCount(); i < max; ++i) {
        const Reference<Cycle> &d = graph.cycle(i);
        if (!d) continue;
        if (!set(laydest(layId+1, graphID+1, i+1), d)) {
            return false;
        }
    }
    return true;
}
bool Mapping::saveCycles(int layId, const Layout &lay)
{
    for (size_t i = 0, max = lay.graphCount(); i < max; ++i) {
        const Item<Graph> &gi = lay.graph(i);
        Graph *g;
        if (!(g = gi)) continue;
        if (!saveCycles(layId, i, *g)) {
            return false;
        }
    }
    return true;
}

// load cycles references
bool Mapping::loadCycles(int layId, int graphID, const Graph &graph) const
{
    if (layId >= UINT16_MAX || graphID >= UINT16_MAX) {
        return false;
    }
    for (size_t i = 0, max = graph.worldCount(); i < max; ++i) {
        Reference<Cycle> *cyc = Map::get(laydest(layId+1, graphID+i, i+1));
        if (!cyc) continue;
        if (!graph.setCycle(i, *cyc)) {
            return false;
        }
    }
    return true;
}
bool Mapping::loadCycles(int layId, const Layout &lay) const
{
    for (size_t i = 0, max = lay.graphCount(); i < max; ++i) {
        const Item<Graph> &gi = lay.graph(i);
        Graph *g;
        if (!(g = gi)) continue;
        if (!loadCycles(layId, i, *g)) {
            return false;
        }
    }
    return true;
}
// deregister cycle references
void Mapping::clearCycles(int lay, int grf, int wld) const
{
    for (size_t i = 0, max = _d.size(); i < max; ++i) {
        Element *e = _d.get(i);
        if (!e) continue;
        if (lay >= 0 && e->key.lay != lay) continue;
        if (grf >= 0 && e->key.grf != grf) continue;
        if (wld >= 0 && e->key.grf != wld) continue;
        e->value = Reference<Cycle>();
    }
}
// get cycle reference
const Reference<Cycle> &Mapping::getCycle(const laydest &dest) const
{
    static const Reference<Cycle> def;
    // search matching destination
    for (int i = 0, max = _d.size(); i < max; ++i) {
        Element *e;
        if (!(e = _d.get(i))) {
            continue;
        }
        // match target cycle
        if (!(e->key == laydest(dest.lay, dest.grf, dest.wld))) {
            continue;
        }
        // cycle not active
        Cycle *c;
        if (!(c = e->value)) {
            return def;
        }
        // bad dimension
        Polyline *pl = c->part(0);
        if (pl && strlen(pl->format()) <= dest.dim) {
            return def;
        }
        // return cycle reference
        return e->value;
    }
    // no destination found
    return def;
}

__MPT_NAMESPACE_END
