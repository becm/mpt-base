/*
 * item grouping implementation
 */

#include <string>

#include "layout.h"

__MPT_NAMESPACE_BEGIN

metatype *item_group::create(const char *type, int nl)
{
	if (!type || !nl) {
		return 0;
	}
	if (nl < 0) {
		nl = strlen(type);
	}
	
	if (nl == 4 && !memcmp("line", type, nl)) {
		return new reference<layout::line>::type;
	}
	if (nl == 4 && !memcmp("text", type, nl)) {
		return new reference<layout::text>::type;
	}
	if (nl == 5 && !memcmp("graph", type, nl)) {
		return new reference<layout::graph>::type;
	}
	if (nl == 5 && !memcmp("world", type, nl)) {
		return new reference<layout::graph::world>::type;
	}
	if (nl == 4 && !memcmp("axis", type, nl)) {
		return new reference<layout::graph::axis>::type;
	}
	class TypedAxis : public reference<layout::graph::axis>::type
	{
	public:
		TypedAxis(int type)
		{ format = type & 0x3; }
	};
	
	if (nl == 5 && !memcmp("xaxis", type, nl)) {
		return new TypedAxis(AxisStyleX);
	}
	if (nl == 5 && !memcmp("yaxis", type, nl)) {
		return new TypedAxis(AxisStyleY);
	}
	if (nl == 5 && !memcmp("zaxis", type, nl)) {
		return new TypedAxis(AxisStyleZ);
	}
	return 0;
}

// group storing elements in item array
int item_group::convert(int type, void *ptr)
{
	int me = type_properties<group *>::id(true);
	
	if (!type) {
		static const uint8_t fmt[] = { TypeCollectionPtr, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me > 0 ? me : TypeArray;
	}
	
	if (assign(static_cast<group *>(this), type, ptr)) {
		return TypeArray;
	}
	if (assign(static_cast<collection *>(this), type, ptr)) {
		return me > 0 ? me : TypeArray;
	}
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		return me > 0 ? me : TypeCollectionPtr;
	}
	return BadType;
}
item_group::~item_group()
{ }

void item_group::unref()
{
	delete this;
}
item_group *item_group::clone() const
{
	item_group *copy = new item_group;
	copy->_items = _items;
	return copy;
}

int item_group::each(item_handler_t fcn, void *ctx) const
{
	int ret = 0;
	for (item<metatype> &it : _items) {
		int curr = fcn(ctx, &it, it.instance(), 0);
		if (curr < 0) {
			return curr;
		}
		ret |= curr;
		if (curr & TraverseStop) {
			return ret;
		}
	}
	return ret;
}
int item_group::append(const identifier *id, metatype *mt)
{
	item<metatype> *it = _items.append(mt, 0);
	if (!it) {
		return BadOperation;
	}
	if (id) {
		*it = *id;
	}
	return _items.count();
}
size_t item_group::clear(const metatype *ref)
{
	long remove = 0;
	if (!ref) {
		remove = _items.length();
		_items = item_array<metatype>();
		return remove ? true : false;
	}
	long empty = 0;
	for (auto &it : _items) {
		metatype *curr = it.instance();
		if (!curr) {
			++empty;
			continue;
		}
		if (curr != ref) {
			continue;
		}
		it.set_instance(nullptr);
		++remove;
	}
	if ((remove + empty) > _items.length()/2) {
		_items.compact();
	}
	return remove;
}
int item_group::bind(const relation *from, logger *out)
{
	int count = 0;
	for (auto &it : _items) {
		metatype *curr;
		group *g;
		if (!(curr = it.instance()) || !(g = typecast<group>(*curr))) {
			continue;
		}
		int ret;
		collection_relation rel(*g, from);
		if ((ret = g->bind(&rel, out)) < 0) {
			return ret;
		}
		count += ret;
	}
	return count;
}

__MPT_NAMESPACE_END
