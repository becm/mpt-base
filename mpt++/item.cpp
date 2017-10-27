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

int Group::typeIdentifier()
{
    static int id = 0;
    if (!id && (id = mpt_valtype_meta_new("group")) < 0) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
// object interface for group
int Group::property(struct property *pr) const
{
    if (!pr) {
        return typeIdentifier();
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
int Group::setProperty(const char *, const metatype *)
{
    return BadOperation;
}
// generic item group
size_t Group::clear(const reference *)
{ return 0; }
Item<metatype> *Group::append(metatype *)
{ return 0; }
const Item<metatype> *Group::item(size_t) const
{ return 0; }
bool Group::bind(const Relation &, logger *)
{ return true; }

bool Group::addItems(node *head, const Relation *relation, logger *out)
{
    const char _func[] = "mpt::Group::addItems";

    for (; head; head = head->next) {
        metatype *from = head->_meta;

        if (from && from->addref()) {
            Reference<metatype> m;
            m.setPointer(from);
            Item<metatype> *it;
            if ((it = append(from))) {
                m.detach();
                it->setName(head->ident.name());
            } else {
                if (out) out->message(_func, out->Warning, "%s %p: %s", MPT_tr("group"), this, MPT_tr("failed to add object"));
            }
            continue;
        }
        // name is property
        const char *name;

        if (!(name = mpt_node_ident(head))) {
            if (out) out->message(_func, out->Error, "%s %p: %s", MPT_tr("node"), head, MPT_tr("bad element name"));
            return false;
        }
        // set property
        value val;
        if (from && (val.ptr = mpt_meta_data(from))) {
            object *obj;
            if ((obj = from->cast<object>())) {
                val.fmt = 0;
                obj->set(name, val, out);
                continue;
            }
            if (out) out->message(_func, out->Warning, "%s (%p): %s", name, from, MPT_tr("parent not an object"));
            continue;
        }
        // get item type
        const char *pos;
        size_t len;
        val.fmt = pos = name;
        name = mpt_convert_key(&pos, 0, &len);
        if (!name || name != val.fmt || !*name) {
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("bad element name"), val.fmt);
            continue;
        }
        // create item
        if (!(from = create(name, len))) {
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("invalid item type"), std::string(name, len).c_str());
            continue;
        }
        // get item name
        name = mpt_convert_key(&pos, ":", &len);

        if (!name || !len) {
            if (out) out->message(_func, out->Warning, "%s", MPT_tr("empty item name"));
            from->unref();
            continue;
        }
        // name conflict on same level
        if (GroupRelation(*this).find(from->type(), name, len)) {
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("conflicting item name"), std::string(name, len).c_str());
            from->unref();
            continue;
        }
        const char *ident = name;
        int ilen = len;

        // find dependant items
        while ((name = mpt_convert_key(&pos, 0, &len))) {
            metatype *curr;
            if (relation) curr = relation->find(from->type(), name, len);
            else curr = GroupRelation(*this).find(from->type(), name, len);
            object *obj, *src;
            if ((src = curr->cast<object>()) && (obj = from->cast<object>())) {
                obj->setProperties(*src, out);
                continue;
            }
            if (out) out->message(_func, out->Error, "%s: %s: %s",
                                  MPT_tr("unable to get inheritance"), head->ident.name(), std::string(name, len).c_str());
            return false;
        }
        // add item to group
        Item<metatype> *ni = append(from);
        if (!ni) {
            from->unref();
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("unable add item"), std::string(ident, ilen).c_str());
            continue;
        }
        ni->setName(ident, ilen);

        // set properties and subitems
        if (!head->children) continue;

        // process child items
        Group *ig;
        if ((ig = from->cast<Group>())) {
            if (!relation) {
                if (!(ig->addItems(head->children, relation, out))) return false;
                continue;
            }
            GroupRelation rel(*ig, relation);
            if (!(ig->addItems(head->children, &rel, out))) return false;
            continue;
        }
        object *obj;
        if (!(obj = from->cast<object>())) {
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
            if (out) out->message(_func, out->Warning, "%s: %s: %s", MPT_tr("bad value type"), ni->name(), name);
        }
    }
    return true;
}
metatype *Group::create(const char *type, int nl)
{
    if (!type || !nl) return 0;

    if (nl < 0) {
        nl = strlen(type);
    }

    if (nl == 4 && !memcmp("line", type, nl)) return new Reference<Line>::instance;

    if (nl == 4 && !memcmp("text", type, nl)) return new Reference<Text>::instance;

    if (nl == 5 && !memcmp("world", type, nl)) return new Reference<World>::instance;

    if (nl == 5 && !memcmp("graph", type, nl)) return new Reference<Graph>::instance;

    if (nl == 4 && !memcmp("axis", type, nl)) return new Reference<Axis>::instance;
    
    class TypedAxis : public Reference<Axis>::instance
    {
    public:
        TypedAxis(int type)
        { format = type & 0x3; }
    };

    if (nl == 5 && !memcmp("xaxis", type, nl)) return new TypedAxis(AxisStyleX);

    if (nl == 5 && !memcmp("yaxis", type, nl)) return new TypedAxis(AxisStyleY);

    if (nl == 5 && !memcmp("zaxis", type, nl)) return new TypedAxis(AxisStyleZ);

    return 0;
}

// group storing elements in RefArray
Collection::~Collection()
{ }

void Collection::unref()
{
    delete this;
}
int Collection::conv(int type, void *ptr) const
{
    int me = Group::typeIdentifier();
    if (me < 0) {
        me = object::Type;
    }
    if (!type) {
        static const char fmt[] = { array::Type };
        if (ptr) *static_cast<const char **>(ptr) = fmt;
        return me;
    }
    if (type == metatype::Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == object::Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const Group **>(ptr) = this;
        return array::Type;
    }
    return BadType;
}
Collection *Collection::clone() const
{
    Collection *copy = new Collection;
    copy->_items = _items;
    return copy;
}

const Item<metatype> *Collection::item(size_t pos) const
{
    return _items.get(pos);
}
Item<metatype> *Collection::append(metatype *mt)
{
    return _items.append(mt, 0);
}
size_t Collection::clear(const reference *ref)
{
    long remove = 0;
    if (!ref) {
        remove = _items.length();
        _items = ItemArray<metatype>();
        return remove ? true : false;
    }
    long empty = 0;
    for (auto &it : _items) {
        reference *curr = it.pointer();
        if (!curr) { ++empty; continue; }
        if (curr != ref) continue;
        it.setPointer(nullptr);
        ++remove;
    }
    if ((remove + empty) > _items.length()/2) {
        _items.compact();
    }
    return remove;
}
bool Collection::bind(const Relation &from, logger *out)
{
    for (auto &it : _items) {
        metatype *curr;
        Group *g;
        if (!(curr = it.pointer()) || !(g = curr->cast<Group>())) {
            continue;
        }
        if (!g->bind(GroupRelation(*g, &from), out)) {
            return false;
        }
    }
    return true;
}


// Relation search operations
metatype *GroupRelation::find(int type, const char *name, int nlen) const
{
    const Item<metatype> *c;
    const char *sep;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    if (_sep && (sep = (char *) memchr(name, _sep, nlen))) {
        size_t plen = sep - name;
        for (int i = 0; (c = _curr.item(i)); ++i) {
            metatype *m = c->pointer();
            if (!m || !c->equal(name, plen)) continue;
            const Group *g;
            if (!(g = m->cast<Group>())) continue;
            if ((m = GroupRelation(*g, this).find(type, sep+1, nlen-plen-1))) {
                return m;
            }
        }
    }
    else {
        for (int i = 0; (c = _curr.item(i)); ++i) {
            metatype *m = c->pointer();
            if (!m || (type && m->type() != type)) continue;
            if (!c->equal(name, nlen)) continue;
            return m;
        }
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

metatype *NodeRelation::find(int type, const char *name, int nlen) const
{
    if (!_curr) return 0;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    for (const node *c = _curr->children; c; c = c->next) {
        metatype *m;
        if (!(m = c->_meta) || !c->ident.equal(name, nlen)) continue;
        if (type && m->type() != type) continue;
        return m;
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

__MPT_NAMESPACE_END
