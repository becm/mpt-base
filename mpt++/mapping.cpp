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
int Mapping::add(const msgbind &src, const msgdest &dst, int client)
{
    const Reference<Cycle> *r;
    if (!(r = getCycle(dst))) {
        return -2;
    }
    mapping map(src, dst, client);
    return mpt_mapping_add(&_bind, &map);
}
// clear data mapping
int Mapping::del(const msgbind *src, const msgdest *dest, int client) const
{
    return mpt_mapping_del(&_bind, src, dest, client);
}
// get data mapping
Array<msgdest> Mapping::destinations(const msgbind &src, int client) const
{
    Array<msgdest> arr;

    for (auto &map : _bind.slice()) {
        if (mpt_mapping_cmp(&map, &src, client)) {
            continue;
        }
        arr.set(arr.length(), map.dest);
    }
    return arr;
}
// clear data mappings
void Mapping::clear()
{
    _bind = Array<mapping>();
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
        if (!set(msgdest(layId+1, graphID+1, i+1), d)) {
            return false;
        }
    }
    return true;
}
bool Mapping::saveCycles(int layId, const Layout &lay)
{
    int i = 0;
    for (auto &it : lay.graphs()) {
        Graph *g;
        if (!(g = it.pointer())) continue;
        if (!saveCycles(layId, i++, *g)) {
            return false;
        }
    }
    return true;
}

// load cycles references
int Mapping::loadCycles(int layId, int graphID, const Graph &graph) const
{
    if (layId >= UINT8_MAX || layId < 0
        || graphID >= UINT8_MAX || layId < 0) {
        return BadValue;
    }
    int total = 0;
    for (size_t i = 0, max = graph.worldCount(); i < max; ++i) {
        Reference<Cycle> *cyc = Map::get(msgdest(layId+1, graphID+1, i+1));
        if (!cyc) continue;
        if (!graph.setCycle(i, *cyc)) {
            return total ? BadOperation : BadArgument;
        }
        ++total;
    }
    return total;
}
int Mapping::loadCycles(int layId, const Layout &lay) const
{
    int i = -1, total = 0;
    for (auto &it : lay.graphs()) {
        Graph *g;
        int curr;
        ++i;
        if (!(g = it.pointer())) continue;
        if ((curr = loadCycles(layId, i, *g)) < 0) {
            return total ? BadOperation : BadArgument;
        }
        total += curr;
    }
    return total;
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
const Reference<Cycle> *Mapping::getCycle(const msgdest &dest) const
{
    // search matching destination
    for (auto &e : _d) {
        // match target cycle
        if (!(e.key == msgdest(dest.lay, dest.grf, dest.wld))) {
            continue;
        }
        // cycle not active
        Cycle *c;
        if (!(c = e.value.pointer())) {
            return 0;
        }
        // bad dimension
        Polyline *pl = c->part(0);
        if (pl && strlen(pl->format()) <= dest.dim) {
            return 0;
        }
        // return cycle reference
        return &e.value;
    }
    // no destination found
    return 0;
}

__MPT_NAMESPACE_END
