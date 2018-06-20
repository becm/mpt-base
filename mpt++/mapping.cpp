/*
 * MPT C++ library
 *   cycle target mapping
 */

#define __STDC_LIMIT_MACROS
#include <limits>

#include "message.h"

#include "layout.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<map<laydest, reference_wrapper<Cycle> >::entry>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<mapping>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<laydest>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}

// add data mapping
int graphic::mapping::add(valsrc src, laydest dst, int client)
{
    const reference_wrapper<Cycle> *r;
    if (!(r = cycle(dst))) {
        return MissingData;
    }
    ::mpt::mapping map(src, dst, client);
    return mpt_mapping_add(&_bind, &map);
}
// clear data mapping
int graphic::mapping::del(const valsrc *src, const laydest *dest, int client) const
{
    return mpt_mapping_del(&_bind, src, dest, client);
}
// get data mapping
typed_array<laydest> graphic::mapping::destinations(valsrc src, int client) const
{
    typed_array<laydest> arr;

    for (auto &map : _bind.elements()) {
        if (mpt_mapping_cmp(&map, &src, client)) {
            continue;
        }
        arr.set(arr.length(), map.dest);
    }
    return arr;
}
// clear data mappings
void graphic::mapping::clear()
{
    _bind = typed_array<::mpt::mapping>();
    _d = typed_array<entry>();
}
// get cycle reference
const reference_wrapper<Cycle> *graphic::mapping::cycle(laydest dst) const
{
    // search matching destination
    for (auto &e : _d) {
        // match target cycle
        if (!e.key.match(dst, laydest::MatchPath)) {
            continue;
        }
        // breach of mapping dimension limit
        if (e.key.dim && e.key.dim < dst.dim) {
            return 0;
        }
        // return cycle reference
        return &e.value;
    }
    // no destination found
    return 0;
}
// set cycle reference
bool graphic::mapping::set_cycle(laydest dst, Cycle *ref)
{
    if (!set(dst, reference_wrapper<Cycle>())) {
        return false;
    }
    if (!ref) {
        return true;
    }
    reference_wrapper<Cycle> *ptr;
    if (!(ptr = get(dst))) {
        return false;
    }
    ptr->set_reference(ref);
    return true;
}

// save cycle references
int graphic::mapping::set_cycles(const span<const reference_wrapper<Layout> > &layouts, update_hint hint)
{
    int total = 0;
    for (size_t i = 0, lmax = layouts.size(); i < lmax; ++i) {
        auto lay = layouts.nth(i)->reference();
        if (!lay) continue;
        if (hint.match & laydest::MatchLayout
         && hint.lay != i) {
            continue;
        }
        const auto graphs = lay->graphs();
        for (size_t j = 0, gmax = graphs.size(); j < gmax; ++j) {
            auto grf = graphs.nth(i)->reference();
            if (!grf) continue;
            if (hint.match & laydest::MatchGraph
             && hint.grf != j) {
                continue;
            }
            const auto worlds = grf->worlds();
            for (size_t k = 0, wmax = worlds.size(); k < wmax; ++k) {
                auto wld = worlds.nth(k)->reference();
                if (!wld) continue;
                if (hint.match & laydest::MatchWorld
                 && hint.wld != k) {
                    continue;
                }
                if (!set(laydest(i, j, k), wld->cycle)) {
                    return total;
                }
                ++total;
            }
        }
    }
    return total;
}
// load cycles references
int graphic::mapping::get_cycles(const span<const reference_wrapper<Layout> > &layouts, update_hint hint)
{
    int total = 0;
    for (size_t i = 0, lmax = layouts.size(); i < lmax; ++i) {
        auto lay = layouts.nth(i)->reference();
        if (!lay) continue;
        if (hint.match & laydest::MatchLayout
         && hint.lay != i) {
            continue;
        }
        const auto graphs = lay->graphs();
        for (size_t j = 0, gmax = graphs.size(); j < gmax; ++j) {
            auto grf = graphs.nth(j)->reference();
            if (!grf) continue;
            if (hint.match & laydest::MatchGraph
             && hint.grf != j) {
                continue;
            }
            int dim = grf->transform().dimensions();
            const auto worlds = grf->worlds();
            for (size_t k = 0, wmax = worlds.size(); k < wmax; ++k) {
                auto wld = worlds.nth(k)->reference();
                if (!wld) continue;
                if (hint.match & laydest::MatchWorld
                 && hint.wld != k) {
                    continue;
                }
                for (entry *b = _d.begin(), *e = _d.end(); b < e; ++b) {
                    if (!b->key.match(laydest(i, j, k), laydest::MatchPath)) {
                        continue;
                    }
                    if (!b->value.reference()) {
                        break;
                    }
                    b->key.dim = dim;
                    wld->cycle = b->value;
                    ++total;
                }
            }
        }
    }
    return total;
}
// deregister cycle references
int graphic::mapping::clear_cycles(update_hint hint) const
{
    int clear = 0;
    for (auto &e : _d) {
        if (hint.match & laydest::MatchLayout && e.key.lay != hint.lay) continue;
        if (hint.match & laydest::MatchGraph  && e.key.grf != hint.grf) continue;
        if (hint.match & laydest::MatchWorld  && e.key.grf != hint.wld) continue;
        if (!e.value.reference()) continue;
        e.value.set_reference(0);
        ++clear;
    }
    return clear;
}

__MPT_NAMESPACE_END
