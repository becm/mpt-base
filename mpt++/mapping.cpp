/*
 * MPT C++ library
 *   cycle target mapping
 */

#define __STDC_LIMIT_MACROS
#include <limits>

#include "message.h"

#include "layout.h"
#include "output.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

// add data mapping
int Mapping::add(const msgbind &src, const laydest &dst, int client)
{
    if (!getCycle(dst).pointer()) {
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
Array<laydest> Mapping::destinations(const msgbind &src, int client) const
{
    const mapping *map = (mapping *) _bind.base();
    size_t len = _bind.used() / sizeof(*map);
    Array<laydest> arr;

    for (size_t i = 0; i < len; ++i) {
        if (mpt_mapping_cmp(map, &src, client)) {
            continue;
        }
        arr.set(arr.size(), map->dest);
    }
    return arr;
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
    if (layId >= UINT8_MAX || layId < 0
        || graphID >= UINT8_MAX || layId < 0) {
        return false;
    }
    for (size_t i = 0, max = graph.worldCount(); i < max; ++i) {
        const Reference<Cycle> &d = graph.cycle(i);
        if (!d.pointer()) continue;
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
        if (!(g = gi.pointer())) continue;
        if (!saveCycles(layId, i, *g)) {
            return false;
        }
    }
    return true;
}

// load cycles references
bool Mapping::loadCycles(int layId, int graphID, const Graph &graph) const
{
    if (layId >= UINT8_MAX || layId < 0
        || graphID >= UINT8_MAX || layId < 0) {
        return false;
    }
    for (size_t i = 0, max = graph.worldCount(); i < max; ++i) {
        Reference<Cycle> *cyc = Map::get(laydest(layId+1, graphID+1, i+1));
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
        if (!(g = gi.pointer())) continue;
        if (!loadCycles(layId, i, *g)) {
            return false;
        }
    }
    return true;
}
// deregister cycle references
void Mapping::clearCycles(int lay, int grf, int wld) const
{
    for (auto &e : _d) {
        if (lay >= 0 && e.key.lay != lay) continue;
        if (grf >= 0 && e.key.grf != grf) continue;
        if (wld >= 0 && e.key.grf != wld) continue;
        e.value = Reference<Cycle>();
    }
}
// get cycle reference
const Reference<Cycle> &Mapping::getCycle(const laydest &dest) const
{
    static const Reference<Cycle> def;
    // search matching destination
    for (auto &e : _d) {
        // match target cycle
        if (!(e.key == laydest(dest.lay, dest.grf, dest.wld))) {
            continue;
        }
        // cycle not active
        Cycle *c;
        if (!(c = e.value.pointer())) {
            return def;
        }
        // bad dimension
        Polyline *pl = c->part(0);
        if (pl && strlen(pl->format()) <= dest.dim) {
            return def;
        }
        // return cycle reference
        return e.value;
    }
    // no destination found
    return def;
}

__MPT_NAMESPACE_END
