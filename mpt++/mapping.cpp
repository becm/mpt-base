/*
 * MPT C++ library
 *   cycle target mapping
 */

#define __STDC_LIMIT_MACROS
#include <limits>

#include "types.h"
#include "message.h"

#include "layout.h"

#include "graphic.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<map<laydest, reference<cycle> >::entry>::id()
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
	const reference<class cycle> *r;
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
const reference<cycle> *graphic::mapping::cycle(laydest dst) const
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
bool graphic::mapping::set_cycle(laydest dst, class cycle *ref)
{
	if (!set(dst, reference<class cycle>())) {
		return false;
	}
	if (!ref) {
		return true;
	}
	reference<class cycle> *ptr;
	if (!(ptr = get(dst))) {
		return false;
	}
	ptr->set_instance(ref);
	return true;
}

// save cycle references
int graphic::mapping::set_cycles(const span<const reference<layout> > &layouts, hint h)
{
	int total = 0;
	for (size_t i = 0, lmax = layouts.size(); i < lmax; ++i) {
		auto lay = layouts.nth(i)->instance();
		if (!lay) {
			continue;
		}
		if (h.match & laydest::MatchLayout
		 && h.lay != i) {
			continue;
		}
		const auto graphs = lay->graphs();
		for (size_t j = 0, gmax = graphs.size(); j < gmax; ++j) {
			auto grf = graphs.nth(j)->instance();
			if (!grf) {
				continue;
			}
			if (h.match & laydest::MatchGraph
			 && h.grf != j) {
				continue;
			}
			const auto worlds = grf->worlds();
			for (size_t k = 0, wmax = worlds.size(); k < wmax; ++k) {
				auto wld = worlds.nth(k)->instance();
				if (!wld) {
					continue;
				}
				if (h.match & laydest::MatchWorld
				 && h.wld != k) {
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
int graphic::mapping::get_cycles(const span<const reference<layout> > &layouts, hint h)
{
	int total = 0;
	for (size_t i = 0, lmax = layouts.size(); i < lmax; ++i) {
		auto lay = layouts.nth(i)->instance();
		if (!lay) {
			continue;
		}
		if (h.match & laydest::MatchLayout
		 && h.lay != i) {
			continue;
		}
		const auto graphs = lay->graphs();
		for (size_t j = 0, gmax = graphs.size(); j < gmax; ++j) {
			auto grf = graphs.nth(j)->instance();
			if (!grf) {
				continue;
			}
			if (h.match & laydest::MatchGraph
			 && h.grf != j) {
				continue;
			}
			int dim = grf->transform().dimensions();
			const auto worlds = grf->worlds();
			for (size_t k = 0, wmax = worlds.size(); k < wmax; ++k) {
				auto wld = worlds.nth(k)->instance();
				if (!wld) {
					continue;
				}
				if (h.match & laydest::MatchWorld
				 && h.wld != k) {
					continue;
				}
				for (entry *b = _d.begin(), *e = _d.end(); b < e; ++b) {
					if (!b->key.match(laydest(i, j, k), laydest::MatchPath)) {
						continue;
					}
					if (!b->value.instance()) {
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
int graphic::mapping::clear_cycles(hint h) const
{
	int clear = 0;
	for (auto &e : _d) {
		if (h.match & laydest::MatchLayout && e.key.lay != h.lay) {
			continue;
		}
		if (h.match & laydest::MatchGraph  && e.key.grf != h.grf) {
			continue;
		}
		if (h.match & laydest::MatchWorld  && e.key.grf != h.wld) {
			continue;
		}
		if (!e.value.instance()) {
			continue;
		}
		e.value.set_instance(0);
		++clear;
	}
	return clear;
}

__MPT_NAMESPACE_END
