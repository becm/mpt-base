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


// generic item group
int Group::property(struct property *pr) const
{
    if (!pr) return Type;
    return pr->name ? BadArgument : BadOperation;
}
int Group::setProperty(const char *name, metatype *)
{ return name ? BadArgument : BadOperation; }

size_t Group::clear(const metatype *)
{ return 0; }
Item<metatype> *Group::append(metatype *)
{ return 0; }
const Item<metatype> *Group::item(size_t) const
{ return 0; }
bool Group::bind(const Relation &, logger *)
{ return true; }


// bind local childs only
bool Group::copy(const Group &from, logger *)
{
    const Item<metatype> *c;
    for (int i = 0; (c = from.item(i)); ++i) {
        const Item<metatype> *t;
        for (int j = 0; (t = item(j)); ++j) {
            if (c == t) { t = 0; break; }
        }
        metatype *m;
        if (!t || !(m = *c) || !(m = m->clone())) continue;
        Item<metatype> *id;
        if ((id = append(m))) {
            id->setName(c->name());
        } else {
            m->unref();
        }
    }
    return true;
}

bool Group::addItems(node *head, const Relation *relation, logger *out)
{
    const char _func[] = "mpt::Group::addItems";

    for (; head; head = head->next) {
        metatype *from;
        if (!(from = head->_meta)) continue;

        if (from->conv(0, 0) > 0 && (from = from->clone())) {
            Item<metatype> *it;
            if ((it = append(from))) {
                it->setName(head->ident.name());
            } else {
                from->unref();
            }
            continue;
        }

        // name is property
        const char *name;

        if (!(name = mpt_node_ident(head))) {
            if (out) out->error(_func, "%s %p: %s", MPT_tr("node"), head, MPT_tr("bad element name"));
            return false;
        }

        // set property
        value val;
        if ((val.ptr = mpt_meta_data(from))) {
            val.fmt = 0;
            set(name, val, out);
            continue;
        }
        // get item type
        const char *pos;
        size_t len;
        val.fmt = pos = name;
        name = mpt_convert_key(&pos, 0, &len);
        if (!name || name != val.fmt || !*name) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("bad object name"), val.fmt);
            continue;
        }

        // create item
        metatype *it = create(name, len);
        if (!it) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("invalid object type"), std::string(name, len).c_str());
            continue;
        }

        // get item name
        name = mpt_convert_key(&pos, ":", &len);

        if (!name || !len) {
            if (out) out->warning(_func, "%s", MPT_tr("empty object name"));
            it->unref();
            continue;
        }

        if (GroupRelation(*this).find(it->type(), name, len)) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("conflicting object name"), std::string(name, len).c_str());
            it->unref();
            continue;
        }
        const char *ident = name;
        int ilen = len;

        object *obj = it->cast<object>();

        // find dependant items
        while (obj && (name = mpt_convert_key(&pos, 0, &len))) {
            metatype *curr;
            if (relation) curr = relation->find(it->type(), name, len);
            else curr = GroupRelation(*this).find(it->type(), name, len);
            object *from;
            if (curr && (from = curr->cast<object>())) {
                obj->setProperties(*from, out);
                continue;
            }
            if (out) out->error(_func, "%s: %s: %s", MPT_tr("unable to get inheritance"), head->ident.name(), std::string(name, len).c_str());
            return false;
        }
        // add item to group
        Item<metatype> *ni = append(it);
        if (!ni) {
            it->unref();
            if (out) out->error(_func, "%s: %s", MPT_tr("unable add item"), std::string(ident, ilen).c_str());
            continue;
        }
        ni->setName(ident, ilen);

        // set properties and subitems
        if (!head->children) continue;

        // process child items
        Group *ig;
        if ((ig = it->cast<Group>())) {
            if (!relation) {
                if (!(ig->addItems(head->children, relation, out))) return false;
                continue;
            }
            GroupRelation rel(*ig, relation);
            if (!(ig->addItems(head->children, &rel, out))) return false;
            continue;
        }
        // no property interface
        if (!obj) continue;

        // load item properties
        for (node *sub = head->children; sub; sub = sub->next) {
            metatype *mt;
            const char *data;

            if (!(mt = sub->_meta)) continue;
            // skip invalid name
            if (!(name = mpt_node_ident(sub)) || !*name) continue;

            // try value conversion
            value *val;
            if ((val = mt->cast<value>())) {
                if (!obj->set(name, *val, out) && out) {
                    out->warning(_func, "%s: %s: %s = <%s>", MPT_tr("failed to assign property"), ni->name(), name, val->fmt);
                }
                continue;
            }
            if (mt->conv('s', &data) >= 0) {
                value txt(0, data);
                if (!obj->set(name, txt, out) && out) {
                    out->warning(_func, "%s: %s: %s = %s", MPT_tr("failed to assign property"), ni->name(), name, txt.ptr);
                }
                continue;
            }
            if (out) out->warning(_func, "%s: %s: %s", MPT_tr("bad value type"), ni->name(), name);
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

const Transform &Group::transform()
{
    static Transform3 gt;
    return gt;
}

// group storing elements in RefArray
Collection::~Collection()
{ }

const Item<metatype> *Collection::item(size_t pos) const
{
    return _items.get(pos);
}

Item<metatype> *Collection::append(metatype *mt)
{
    return _items.append(mt, 0);
}

size_t Collection::clear(const metatype *mt)
{
    size_t remove = 0;
    if (!mt) {
        remove = _items.size();
        _items = ItemArray<metatype>();
        return remove ? true : false;
    }
    size_t empty = 0;
    for (auto &it : _items) {
        metatype *ref = it;
        if (!ref) { ++empty; continue; }
        if (mt != ref) continue;
        it.detach()->unref();
        ++remove;
    }
    if ((remove + empty) > _items.size()/2) {
        _items.compact();
    }
    return remove;
}
bool Collection::bind(const Relation &from, logger *out)
{
    for (auto &it : _items) {
        metatype *m;
        Group *g;
        if (!(m = it) || !(g = m->cast<Group>())) {
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
            const Group *g;
            metatype *m;
            if (!c || !(m = *c)) continue;
            if (!(g = m->cast<Group>())) continue;
            if (!c->equal(name, plen)) continue;
            if ((m = GroupRelation(*g, this).find(type, sep+1, nlen-plen-1))) {
                return m;
            }
        }
    }
    else {
        for (int i = 0; (c = _curr.item(i)); ++i) {
            metatype *m;
            if (!c || !(m = *c) || !c->equal(name, nlen)) continue;
            if (type) {
                object *obj = m->cast<object>();
                if (!obj || (obj->type() != type)) continue;
            }
            return m;
        }
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

metatype *NodeRelation::find(int type, const char *name, int nlen) const
{
    if (!_curr) return 0;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    metatype *m;
    for (const node *c = _curr->children; c; c = c->next) {
        if (!(m = c->_meta) || !c->ident.equal(name, nlen)) continue;
        if (type) {
            object *obj = m->cast<object>();
            if (!obj || (obj->type() != type)) continue;
        }
        return m;
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

__MPT_NAMESPACE_END
