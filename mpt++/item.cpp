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
// object interface for group
int group::property(struct property *pr) const
{
    if (!pr) {
        return typeinfo<group>::id();
    }
    if (!pr->name || *pr->name) {
        return BadArgument;
    }
    pr->name = "group";
    pr->desc = "mpt item group";
    pr->val.fmt = 0;
    pr->val.ptr = 0;
    return 0;
}
int group::set_property(const char *, const metatype *)
{
    return BadOperation;
}
// generic item group
size_t group::clear(const reference *)
{
    return 0;
}
item<metatype> *group::append(metatype *)
{
    return 0;
}
const item<metatype> *group::item(size_t) const
{
    return 0;
}
bool group::bind(const relation &, logger *)
{
    return true;
}

bool group::add_items(node *head, const relation *relation, logger *out)
{
    const char _func[] = "mpt::group::add_items";

    for (; head; head = head->next) {
        metatype *from = head->_meta;

        if (from && from->addref()) {
            reference_wrapper<metatype> m;
            m.set_instance(from);
            ::mpt::item<metatype> *it;
            if ((it = append(from))) {
                m.detach();
                it->set_name(head->ident.name());
            }
            else if (out) {
                out->message(_func, out->Warning, "%s %p: %s",
                             MPT_tr("group"), this, MPT_tr("failed to add object"));
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
        value val;
        if (from && (val.ptr = mpt_meta_data(from))) {
            if (set(name, val, out)) {
                continue;
            }
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
        if (!(from = create(name, len))) {
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
        if (group_relation(*this).find(from->type(), name, len)) {
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
            if (relation) curr = relation->find(from->type(), name, len);
            else curr = group_relation(*this).find(from->type(), name, len);
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
        ::mpt::item<metatype> *ni = append(from);
        if (!ni) {
            from->unref();
            if (out) {
                out->message(_func, out->Error, "%s: %s",
                             MPT_tr("unable add item"), std::string(ident, ilen).c_str());
            }
            continue;
        }
        ni->set_name(ident, ilen);

        // set properties and subitems
        if (!head->children) continue;

        // process child items
        group *ig;
        if ((ig = from->cast<group>())) {
            if (!relation) {
                if (!(ig->add_items(head->children, relation, out))) {
                    return false;
                }
                continue;
            }
            group_relation rel(*ig, relation);
            if (!(ig->add_items(head->children, &rel, out))) {
                return false;
            }
            continue;
        }
        object *obj;
        if (!(obj = from->cast<object>())) {
            if (out && head->children) {
                out->message(_func, out->Warning, "%s (%p): %s",
                             name, from, MPT_tr("element not an object"));
            }
            continue;
        }
        // load item properties
        for (node *sub = head->children; sub; sub = sub->next) {
            metatype *mt;
            const char *data;

            if (!(mt = sub->_meta)) {
                continue;
            }
            // skip invalid name
            if (!(name = mpt_node_ident(sub)) || !*name) {
                continue;
            }
            // try value conversion
            value val;
            if (mt->conv(value::Type, &val) >= 0) {
                if (obj->set(name, val, out)) {
                    continue;
                }
            }
            if (mt->conv('s', &data) >= 0) {
                if (obj->set(name, data, out)) {
                    continue;
                }
            }
            if (out) {
                out->message(_func, out->Warning, "%s: %s: %s",
                             MPT_tr("bad value type"), ni->name(), name);
            }
        }
    }
    return true;
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
        return new reference_wrapper<layout::line>::type;
    }
    if (nl == 4 && !memcmp("text", type, nl)) {
        return new reference_wrapper<layout::text>::type;
    }
    if (nl == 5 && !memcmp("graph", type, nl)) {
        return new reference_wrapper<layout::graph>::type;
    }
    if (nl == 5 && !memcmp("world", type, nl)) {
        return new reference_wrapper<layout::graph::world>::type;
    }
    if (nl == 4 && !memcmp("axis", type, nl)) {
        return new reference_wrapper<layout::graph::axis>::type;
    }
    class TypedAxis : public reference_wrapper<layout::graph::axis>::type
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

// group storing elements in RefArray
collection::~collection()
{ }

void collection::unref()
{
    delete this;
}
int collection::conv(int type, void *ptr) const
{
    int me = typeinfo<group>::id();
    if (me < 0) {
        me = object::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const group **>(ptr) = this;
        return array::Type;
    }
    if (!type) {
        static const char fmt[] = { array::Type };
        if (ptr) *static_cast<const char **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(metatype::Type)) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return me;
    }
    return BadType;
}
collection *collection::clone() const
{
    collection *copy = new collection;
    copy->_items = _items;
    return copy;
}

const item<metatype> *collection::item(size_t pos) const
{
    return _items.get(pos);
}
item<metatype> *collection::append(metatype *mt)
{
    return _items.append(mt, 0);
}
size_t collection::clear(const reference *ref)
{
    long remove = 0;
    if (!ref) {
        remove = _items.length();
        _items = item_array<metatype>();
        return remove ? true : false;
    }
    long empty = 0;
    for (auto &it : _items) {
        reference *curr = it.instance();
        if (!curr) { ++empty; continue; }
        if (curr != ref) continue;
        it.set_instance(nullptr);
        ++remove;
    }
    if ((remove + empty) > _items.length()/2) {
        _items.compact();
    }
    return remove;
}
bool collection::bind(const relation &from, logger *out)
{
    for (auto &it : _items) {
        metatype *curr;
        group *g;
        if (!(curr = it.instance()) || !(g = curr->cast<group>())) {
            continue;
        }
        if (!g->bind(group_relation(*g, &from), out)) {
            return false;
        }
    }
    return true;
}


// Relation search operations
metatype *group_relation::find(int type, const char *name, int nlen) const
{
    const class item<metatype> *c;
    const char *sep;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    if (_sep && name && (sep = (char *) memchr(name, _sep, nlen))) {
        size_t plen = sep - name;
        for (int i = 0; (c = _curr.item(i)); ++i) {
            metatype *m = c->instance();
            if (!m || !c->equal(name, plen)) {
                continue;
            }
            const group *g;
            if (!(g = m->cast<group>())) {
                continue;
            }
            if ((m = group_relation(*g, this).find(type, sep + 1, nlen - plen - 1))) {
                return m;
            }
        }
    }
    else {
        for (int i = 0; (c = _curr.item(i)); ++i) {
            metatype *m;
            if (!(m = c->instance())) {
                continue;
            }
            if (name && !c->equal(name, nlen)) {
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
                if (type && type != val && (val = m->conv(type, 0)) < 0) {
                    continue;
                }
            }
            else if (!val) {
                continue;
            }
            return m;
        }
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

metatype *node_relation::find(int type, const char *name, int nlen) const
{
    if (!_curr) return 0;

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
            if (type && type != val && (val = m->conv(type, 0)) < 0) {
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
