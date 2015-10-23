/*
 * item grouping implementation
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/uio.h>

#include "node.h"
#include "config.h"
#include "message.h"
#include "convert.h"
#include "array.h"

#include "layout.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

identifier::identifier(size_t total)
{
    mpt_identifier_init(this, total);
}
bool identifier::equal(const char *name, int nlen) const
{
    return mpt_identifier_compare(this, name, nlen) ? false : true;
}
bool identifier::setName(const char *name, int nlen)
{
    if (nlen < 0) {
        nlen = name ? strlen(name) : 0;
    }
    return (mpt_identifier_set(this, name, nlen)) ? true : false;
}
const char *identifier::name() const
{
    return (mpt_identifier_len(this) <= 0) ? 0 : (const char *) mpt_identifier_data(this);
}
Slice<const char> identifier::data() const
{
    const char *id = (const char *) mpt_identifier_data(this);
    return Slice<const char>(id, _len);
}
// generic item group
size_t Group::clear(const metatype *)
{ return 0; }
Item<metatype> *Group::append(metatype *)
{ return 0; }
const Item<metatype> *Group::item(size_t) const
{ return 0; }
bool Group::bind(const Relation &, logger *)
{ return true; }
bool Group::set(const property &, logger *)
{ return false; }


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
        if (!t || !(m = *c)) continue;
        Reference<metatype> ref(m->addref());
        Item<metatype> *id;
        if ((id = append(ref))) {
            id->setName(c->name());
            ref.detach();
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

        if (from->type()) {
            Reference<metatype> ref(from->addref());
            Item<metatype> *it;
            if (ref && (it = append(ref))) {
                ref.detach();
                it->setName(head->ident.name());
            }
            continue;
        }


        // name is property
        property pr(mpt_node_ident(head));

        if (!pr.name) {
            pr.name = "";
            pr.desc = 0;
            if (from->property(&pr) < 0 || !pr.name) {
                pr.name = "<unknown>";
            }
            if (out) out->error(_func, "%s: ", MPT_tr("bad element name"), pr.name);
            return false;
        }

        // set property
        if ((pr.val.ptr = (const char *) mpt_meta_data(from))) {
            pr.desc = 0;
            pr.val.fmt = 0;
            set(pr, out);
            continue;
        }
        // get item type
        size_t len;
        pr.val.fmt = pr.desc = pr.name;
        pr.name = mpt_convert_key(&pr.desc, 0, &len);
        if (!pr.name || !*pr.name) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("bad object name"), pr.val.fmt);
            continue;
        }

        // create item
        metatype *it = create(pr.name, len);
        if (!it) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("invalid object type"), std::string(pr.name, len).c_str());
            continue;
        }

        // get item name
        pr.name = mpt_convert_key(&pr.desc, ":", &len);

        if (!pr.name || !len) {
            if (out) out->warning(_func, "%s", MPT_tr("empty object name"));
            it->unref();
            continue;
        }

        if (GroupRelation(*this).find(it->type(), pr.name, len)) {
            if (out) out->warning(_func, "%s: %s", MPT_tr("conflicting object name"), std::string(pr.name, len).c_str());
            it->unref();
            continue;
        }
        const char *name = pr.name;
        int nlen = len;

        // find dependant items
        while ((pr.name = mpt_convert_key(&pr.desc, 0, &len))) {
            metatype *curr;
            if (relation) curr = relation->find(it->type(), pr.name, len);
            else curr = GroupRelation(*this).find(it->type(), pr.name, len);
            if (curr) {
                it->setProperties(*curr, out);
                continue;
            }
            if (out) out->error(_func, "%s: %s: %s", MPT_tr("unable to get inheritance"), head->ident.name(), std::string(pr.name, len).c_str());
            return false;
        }


        // replace node metadata and update Item
        Item<metatype> *ni = append(it);
        if (!ni) {
            it->unref();
            if (out) out->error(_func, "%s: %s", MPT_tr("unable add item"), std::string(name, nlen).c_str());
            continue;
        }
        ni->setName(name, nlen);

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
        // load item properties
        for (node *sub = head->children; sub; sub = sub->next) {
            metatype *mt;
            property pr;
            if (!(mt = sub->_meta) || mt->property(&pr) < 0 || !pr.val.ptr) continue;
            // skip invalid configuration
            if (!(pr.name = mpt_node_ident(sub)) || !*pr.name || it->set(pr, out) || !out) continue;
            // error handling
            if (!pr.val.ptr) {
                if (out) out->warning(_func, "%s: %s: %s", MPT_tr("bad property"), ni->name(), pr.name);
            } else {
                if (out) out->warning(_func, "%s: %s: %s = %s", MPT_tr("bad property value"), ni->name(), pr.name, pr.val.ptr);
            }
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

    if (nl == 4 && !memcmp("line", type, nl)) return new Line;

    if (nl == 4 && !memcmp("text", type, nl)) return new Text;

    if (nl == 5 && !memcmp("world", type, nl)) return new World;

    if (nl == 5 && !memcmp("graph", type, nl)) return new Graph;

    if (nl == 4 && !memcmp("axis", type, nl)) return new Axis;

    if (nl == 5 && !memcmp("xaxis", type, nl)) return new Axis(AxisStyleX);

    if (nl == 5 && !memcmp("yaxis", type, nl)) return new Axis(AxisStyleY);

    if (nl == 5 && !memcmp("zaxis", type, nl)) return new Axis(AxisStyleZ);

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

int Collection::unref()
{ delete this; return 0; }

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
            if (!c || !(m = *c)) continue;
            if (type && (m->type() != type)) continue;
            if (c->equal(name, nlen)) {
                return m;
            }
        }
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

metatype *NodeRelation::find(int type, const char *name, int nlen) const
{
    if (!_curr) return 0;

    if (nlen < 0) nlen = name ? strlen(name) : 0;

    metatype *m;
    for (const node *curr = _curr->children; curr; curr = curr->next) {
        if (!(m = curr->_meta)) continue;
        if (type && (m->type()) != type) continue;
        if (!curr->ident.equal(name, nlen)) return m;
    }
    return _parent ? _parent->find(type, name, nlen) : 0;
}

__MPT_NAMESPACE_END
