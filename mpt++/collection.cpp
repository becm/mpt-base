/*
 * item grouping implementation
 */

#include <cstring>

#include "node.h"
#include "convert.h"
#include "object.h"

#include "collection.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptCollection
 * \brief get collection interface traits
 * 
 * Get named traits for collection pointer data.
 * 
 * \return named traits for collection pointer
 */
const named_traits *collection::pointer_traits()
{
	return mpt_interface_traits(TypeCollectionPtr);
}

/*!
 * \ingroup mptCollection
 * \brief get group interface traits
 * 
 * Get named traits for group pointer data.
 * 
 * \return named traits for group pointer
 */
const named_traits *group::pointer_traits()
{
	static const named_traits *traits = 0;
	if (!traits && !(traits = mpt_type_interface_add("mpt.group"))) {
		traits = mpt_type_interface_add(0);
	}
	return traits;
}

template <> int type_properties<group *>::id(bool obtain) {
	static const named_traits *traits = 0;
	if (traits) {
		return traits->type;
	}
	if (obtain && !(traits = group::pointer_traits())) {
		return BadOperation;
	}
	return traits ? traits->type : static_cast<int>(BadType);
}

template <> const struct type_traits *type_properties<group *>::traits()
{
	static const struct type_traits *traits = 0;
	if (!traits) {
		const named_traits *nt = group::pointer_traits();
		if (nt) {
			traits = &nt->traits;
		}
		else {
			static const struct type_traits fallback(sizeof(group *));
			traits = &fallback;
		}
	}
	return traits;
}

bool add_items(metatype &to, const node *head, const relation *relation, logger *out)
{
	const char _func[] = "mpt::add_items";
	group *grp = to;
	object *obj = to;
	
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
				
				if (from->get(val)) {
					if (obj->set(name, val, out)) {
						continue;
					}
				}
				obj->set(name, from->string(), out);
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
			if ((src = *curr) && (obj = *from)) {
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
		object *curr;
		if (!(curr = *from)) {
			if (out) {
				out->message(_func, out->Warning, "%s (%p): %s",
				             name, from, MPT_tr("element not an object"));
			}
		}
		group *ig;
		if (!relation || !(ig = *from)) {
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
		sub = *mt;
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
