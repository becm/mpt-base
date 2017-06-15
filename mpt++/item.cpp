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
int Group::setProperty(const char *name, const metatype *)
{ return name ? BadArgument : BadOperation; }

size_t Group::clear(const unrefable *)
{ return 0; }
Item<object> *Group::append(object *)
{ return 0; }
const Item<object> *Group::item(size_t) const
{ return 0; }
bool Group::bind(const Relation &, logger *)
{ return true; }

void *Group::toType(int type)
{
    static const char types[] = { object::Type, Group::Type, 0 };
    switch (type) {
      case 0: return (void *) types;
      case object::Type: return static_cast<object *>(this);
      case Group::Type: return static_cast<object *>(this);
      default: return 0;
    }
}
bool Group::addItems(node *head, const Relation *relation, logger *out)
{
    const char _func[] = "mpt::Group::addItems";

    for (; head; head = head->next) {
        metatype *from = head->_meta;

        object *obj;
        if (from && from->conv(object::Type, &obj) > 0 && obj) {
            Reference<object> ro;
            if (!obj->addref()) {
                if (out) out->message(_func, out->Error, "%s %p: %s", MPT_tr("object"), obj, MPT_tr("failed to raise object reference"));
                continue;
            }
            ro.setPointer(obj);
            Item<object> *it;
            if ((it = append(obj))) {
                ro.detach();
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
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("bad object name"), val.fmt);
            continue;
        }

        // create item
        if (!(obj = create(name, len))) {
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("invalid object type"), std::string(name, len).c_str());
            continue;
        }

        // get item name
        name = mpt_convert_key(&pos, ":", &len);

        if (!name || !len) {
            if (out) out->message(_func, out->Warning, "%s", MPT_tr("empty object name"));
            obj->unref();
            continue;
        }
        // name conflict on same level
        if (GroupRelation(*this).find(obj->type(), name, len)) {
            if (out) out->message(_func, out->Warning, "%s: %s", MPT_tr("conflicting object name"), std::string(name, len).c_str());
            obj->unref();
            continue;
        }
        const char *ident = name;
        int ilen = len;

        // find dependant items
        while ((name = mpt_convert_key(&pos, 0, &len))) {
            object *curr;
            if (relation) curr = relation->find(obj->type(), name, len);
            else curr = GroupRelation(*this).find(obj->type(), name, len);
            if (curr) {
                obj->setProperties(*curr, out);
                continue;
            }
            if (out) out->message(_func, out->Error, "%s: %s: %s",
                                  MPT_tr("unable to get inheritance"), head->ident.name(), std::string(name, len).c_str());
            return false;
        }
        // add item to group
        Item<object> *ni = append(obj);
        if (!ni) {
            obj->unref();
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("unable add item"), std::string(ident, ilen).c_str());
            continue;
        }
        ni->setName(ident, ilen);

        // set properties and subitems
        if (!head->children) continue;

        // process child items
        if (obj->property(0) == Group::Type) {
            Group *ig = static_cast<Group *>(obj);
            if (!relation) {
                if (!(ig->addItems(head->children, relation, out))) return false;
                continue;
            }
            GroupRelation rel(*ig, relation);
            if (!(ig->addItems(head->children, &rel, out))) return false;
            continue;
        }
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
                    out->message(_func, out->Warning, "%s: %s: %s = <%s>", MPT_tr("failed to assign property"), ni->name(), name, val->fmt);
                }
                continue;
            }
            if (mt->conv('s', &data) >= 0) {
                value txt(0, data);
                if (!obj->set(name, txt, out) && out) {
                    out->message(_func, out->Warning, "%s: %s: %s = %s", MPT_tr("failed to assign property"), ni->name(), name, txt.ptr);
                }
                continue;
            }
            if (out) out->message(_func, out->Warning, "%s: %s: %s", MPT_tr("bad value type"), ni->name(), name);
        }
    }
    return true;
}
object *Group::create(const char *type, int nl)
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

const Item<object> *Collection::item(size_t pos) const
{
    return _items.get(pos);
}

Item<object> *Collection::append(object *mt)
{
    return _items.append(mt, 0);
}

size_t Collection::clear(const unrefable *mt)
{
    long remove = 0;
    if (!mt) {
        remove = _items.length();
        _items = ItemArray<object>();
        return remove ? true : false;
    }
    long empty = 0;
    for (auto &it : _items) {
        unrefable *ref = it.pointer();
        if (!ref) { ++empty; continue; }
        if (mt != ref) continue;
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
        object *o;
        Group *g;
        if (!(o = it.pointer()) || o->property(0) != Group::Type) {
            continue;
        }
        g = static_cast<Group *>(o);
        if (!g->bind(GroupRelation(*g, &from), out)) {
            return false;
        }
    }
    return true;
}


// Relation search operations
object *GroupRelation::find(int type, const char *name, int nlen) const
{
    const Item<object> *c;
    const char *sep;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    if (_sep && (sep = (char *) memchr(name, _sep, nlen))) {
        size_t plen = sep - name;
        for (int i = 0; (c = _curr.item(i)); ++i) {
            object *o = c->pointer();
            if (!o || !c->equal(name, plen)) continue;
            const Group *g;
            if (o->property(0) != Group::Type) continue;
            g = static_cast<Group *>(o);
            if ((o = GroupRelation(*g, this).find(type, sep+1, nlen-plen-1))) {
                return o;
            }
        }
    }
    else {
        for (int i = 0; (c = _curr.item(i)); ++i) {
            object *o = c->pointer();
            if (!o || !c->equal(name, nlen)) continue;
            if (type && o->type() != type) continue;
            return o;
        }
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

object *NodeRelation::find(int type, const char *name, int nlen) const
{
    if (!_curr) return 0;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    for (const node *c = _curr->children; c; c = c->next) {
        metatype *m;
        if (!(m = c->_meta) || !c->ident.equal(name, nlen)) continue;
        object *o = m->cast<object>();
        if (!o) continue;
        if (type && o->type() != type) continue;
        return o;
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

__MPT_NAMESPACE_END
