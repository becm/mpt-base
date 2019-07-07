/*
 * item grouping implementation
 */

#include <string>

#include <sys/uio.h>

#include "node.h"
#include "config.h"
#include "message.h"
#include "convert.h"
#include "array.h"
#include "types.h"

#include "layout.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<group>::id()
{
	static int id = 0;
	if (!id && (id = mpt_type_interface_new("group")) < 0) {
		id = mpt_type_interface_new(0);
	}
	return id;
}

bool add_items(metatype &to, const node *head, const relation *relation, logger *out)
{
	const char _func[] = "mpt::add_items";
	group *grp = to.cast<group>();
	object *obj = to.cast<object>();
	
	// no assignable target
	if (!grp && !obj) {
		return true;
	}
	
	for (; head; head = head->next) {
		metatype *from = head->_meta;
		
		if (grp && from && from->addref()) {
			reference<metatype> m;
			m.set_instance(from);
			if (grp->append(&head->ident, from) < 0) {
				m.detach();
			}
			else if (out) {
				out->message(_func, out->Warning, "%s %p: %s",
				             MPT_tr("group"), grp, MPT_tr("failed to add object"));
			}
			continue;
		}
		// name is property
		const char *name;
		
		if (!(name = mpt_node_ident(head))) {
			if (out) {
				out->message(_func, out->Error, "%s %p: %s",
				             MPT_tr("node"), head, MPT_tr("bad element name"));
			}
			return false;
		}
		// set property
		if (obj) {
			if (!grp) {
				if (obj->set_property(name, from) >= 0) {
					continue;
				}
			}
			if (from) {
				value val;
				const char *data;
				
				if (from->convert(value::Type, &val) >= 0) {
					if (obj->set(name, val, out)) {
						continue;
					}
				}
				data = mpt_convertable_data(from);
				obj->set(name, data, out);
				continue;
			}
		}
		if (!grp) {
			continue;
		}
		// get item type
		const char *start, *pos;
		size_t len;
		start = pos = name;
		name = mpt_convert_key(&pos, 0, &len);
		if (!name || name != start || !*name) {
			if (out) {
				out->message(_func, out->Warning, "%s: %s",
				             MPT_tr("bad element name"), start);
			}
			continue;
		}
		// create item
		if (!(from = grp->create(name, len))) {
			if (out) {
				out->message(_func, out->Warning, "%s: %s",
				             MPT_tr("invalid item type"), std::string(name, len).c_str());
			}
			continue;
		}
		// get item name
		name = mpt_convert_key(&pos, ":", &len);
		
		if (!name || !len) {
			if (out) {
				out->message(_func, out->Warning, "%s",
				             MPT_tr("empty item name"));
			}
			from->unref();
			continue;
		}
		// name conflict on same level
		if (collection_relation(*grp).find(from->type(), name, len)) {
			if (out) {
				out->message(_func, out->Warning, "%s: %s",
				             MPT_tr("conflicting item name"), std::string(name, len).c_str());
			}
			from->unref();
			continue;
		}
		const char *ident = name;
		int ilen = len;
		
		// find dependant items
		while ((name = mpt_convert_key(&pos, 0, &len))) {
			metatype *curr;
			if (relation) {
				curr = relation->find(from->type(), name, len);
			} else {
				curr = collection_relation(*grp).find(from->type(), name, len);
			}
			if (!curr) {
				if (out) {
					out->message(_func, out->Error, "%s: '%s': %s",
					             MPT_tr("unable to get parent"), head->ident.name(), std::string(name, len).c_str());
				}
				from->unref();
				return false;
			}
			object *obj, *src;
			if ((src = curr->cast<object>()) && (obj = from->cast<object>())) {
				obj->set(*src, out);
				continue;
			}
			from->unref();
			if (out) {
				out->message(_func, out->Error, "%s: '%s': %s",
				             MPT_tr("unable to get inheritance"), head->ident.name(), std::string(name, len).c_str());
			}
			return false;
		}
		// add item to group
		identifier id;
		if (!id.set_name(ident, ilen)) {
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("unable add item"), std::string(ident, ilen).c_str());
			}
			continue;
		}
		if (grp->append(&id, from) < 0) {
			from->unref();
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("unable add item"), std::string(ident, ilen).c_str());
			}
			continue;
		}
		// set properties and subitems
		if (!head->children) {
			continue;
		}
		// process child items
		group *ig = from->cast<group>();
		
		if (!from->cast<object>()) {
			if (out) {
				out->message(_func, out->Warning, "%s (%p): %s",
				             name, from, MPT_tr("element not an object"));
			}
		}
		if (!relation || !ig) {
			if (!(add_items(*from, head->children, 0, out))) {
				return false;
			}
			continue;
		}
		collection_relation rel(*ig, relation);
		if (!(add_items(*from, head->children, &rel, out))) {
			return false;
		}
	}
	return true;
}
// generic item group
int group::bind(const relation *, logger *)
{
	return 0;
}
metatype *group::create(const char *type, int nl)
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
	int me = typeinfo<group>::id();
	if (me < 0) {
		me = metatype::Type;
	}
	else if (type == to_pointer_id(me)) {
		if (ptr) *static_cast<group **>(ptr) = this;
		return array::Type;
	}
	if (!type) {
		static const uint8_t fmt[] = { collection::Type, array::Type, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	if (type == to_pointer_id(collection::Type)) {
		if (ptr) *static_cast<collection **>(ptr) = this;
		return me;
	}
	if (type == to_pointer_id(metatype::Type)) {
		if (ptr) *static_cast<metatype **>(ptr) = this;
		return me;
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
		if (!(curr = it.instance()) || !(g = curr->cast<group>())) {
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


// Relation search operations
struct item_match
{
	metatype *mt;
	const char *ident;
	size_t curr;
	size_t left;
	int type;
	char sep;
};
static int find_item(void *ptr, const identifier *id, metatype *mt, const collection *sub)
{
	struct item_match *ctx = static_cast<item_match *>(ptr);
	if (!id) {
		return 0;
	}
	if (mt && !ctx->left && ctx->type >= 0) {
		int val = mt->type();
		if (!ctx->type) {
			if (!val) {
				return 0;
			}
		}
		if (val != ctx->type && (val = mt->convert(ctx->type, 0)) < 0) {
			return 0;
		}
	}
	if (!id->equal(ctx->ident, ctx->curr)) {
		return 0;
	}
	if (!sub && mt) {
		sub = mt->cast<collection>();
	}
	if (!ctx->left) {
		ctx->mt = mt;
		return TraverseStop | (sub ? TraverseLeafs : TraverseNonLeafs);
	}
	if (!sub) {
		return TraverseLeafs;
	}
	struct item_match tmp = *ctx;
	
	const char *next = ctx->ident + ctx->left;
	const char *sep = (char *) memchr(next, ctx->sep, ctx->left);
	
	if (!sep) {
		return BadValue;
	}
	tmp.curr = sep - next;
	tmp.left -= (tmp.curr + 1);
	tmp.ident = next;
	
	int ret = sub->each(find_item, &tmp);
	if (ret < 0) {
		return ret;
	}
	ret |= TraverseNonLeafs;
	return ret;
}
    
metatype *collection_relation::find(int type, const char *name, int nlen) const
{
	struct item_match m;
	const char *sep;
	
	if (nlen < 0) nlen = name ? strlen(name) : 0;
	
	m.mt = 0;
	m.ident = name;
	m.curr = nlen;
	m.left = 0;
	if (_sep && name && (sep = (char *) memchr(name, _sep, nlen))) {
		size_t plen = sep - name;
		m.curr = plen;
		m.left = nlen - plen - 1;
	}
	m.type = type;
	m.sep = _sep;
	
	if (_curr.each(find_item, &m) >= 0 && m.mt) {
		return m.mt;
	}
	return _parent ? _parent->find(type, name, nlen) : 0;
}

metatype *node_relation::find(int type, const char *name, int nlen) const
{
	if (!_curr) {
		return 0;
	}
	if (nlen < 0) nlen = name ? strlen(name) : 0;

	for (const node *c = _curr->children; c; c = c->next) {
		metatype *m;
		if (!(m = c->_meta)) {
			continue;
		}
		if (name && !c->ident.equal(name, nlen)) {
			continue;
		}
		if (type < 0) {
			return m;
		}
		int val;
		if ((val = m->type()) < 0) {
			continue;
		}
		if (type) {
			if (type != val && (val = m->convert(type, 0)) < 0) {
				continue;
			}
		}
		else if (!val) {
			continue;
		}
		return m;
	}
	return _parent ? _parent->find(type, name, nlen) : 0;
}

__MPT_NAMESPACE_END
