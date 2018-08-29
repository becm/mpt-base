/*
 * graph item implementation
 */

#include <string>

#include "array.h"
#include "convert.h"
#include "node.h"
#include "parse.h"
#include "config.h"

#include "layout.h"

__MPT_NAMESPACE_BEGIN

template class reference<layout::graph>;
template class reference<layout::graph::axis>;
template class reference<layout::graph::world>;
template class reference<layout::graph::data>;
template class reference<class layout::graph::transform>;

template <> int typeinfo<layout::graph>::id()
{
    static int id = 0;
    if (!id && (id = mpt_type_meta_new("graph")) < 0) {
        id = mpt_type_meta_new(0);
    }
    return id;
}

template <> int typeinfo<layout::graph::axis>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<layout::graph::world>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<layout::graph::data>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}

// graph data operations
graph::graph()
{
    mpt_graph_init(this);
}
graph::graph(const graph & gr)
{
    mpt_graph_init(this, &gr);
}
graph & graph::operator= (const graph & gr)
{
    mpt_graph_fini(this);
    mpt_graph_init(this, &gr);
    return *this;
}
graph::~graph()
{
    mpt_graph_fini(this);
}

layout::graph::graph(const ::mpt::graph *from)
{
    if (!from) return;
    *static_cast< ::mpt::graph *>(this) = *from;
}
layout::graph::~graph()
{ }
// metatype interface
int layout::graph::conv(int type, void *ptr) const
{
    int me = typeinfo<graph>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const graph **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            ::mpt::graph::Type, color::Type, 0
            
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(::mpt::graph::Type)) {
        if (ptr) *static_cast<const ::mpt::graph **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<color *>(ptr) = fg;
        return me;
    }
    return collection::conv(type, ptr);
}
// object interface
int layout::graph::property(struct property *prop) const
{
    if (!prop) {
        return typeinfo<graph>::id();
    }
    return mpt_graph_get(this, prop);
}
int layout::graph::set_property(const char *prop, const metatype *src)
{
    int ret = mpt_graph_set(this, prop, src);

    if (ret < 0 || !src) {
        return ret;
    }
    update_transform();
    return ret;
}
// graph group interface
static layout::graph::axis *make_axis(metatype *mt, logger *out, const char *_func, const char *name, int len)
{
    ::mpt::axis *d;
    if ((d = mt->cast< ::mpt::axis>())) {
        return new layout::graph::axis(d);
    }
    object *o;
    if ((o = mt->cast<object>())) {
        layout::graph::axis *a = new layout::graph::axis;
        if (a->set(*o, out)) {
             return a;
        }
        a->unref();
    }
    if (out) {
        const char *msg = MPT_tr("unable to get axis information");
        if (!name || !*name || len == 0) {
            out->message(_func, out->Warning, "%s", msg);
        } else if (len < 0) {
            out->message(_func, out->Warning, "%s: %s", msg, name);
        } else {
            out->message(_func, out->Warning, "%s: %s", msg, std::string(name, len).c_str());
        }
    }
    return 0;
}
static layout::graph::world *make_world(metatype *mt, logger *out, const char *_func, const char *name, int len)
{
    ::mpt::world *d;
    if ((d = mt->cast< ::mpt::world>())) {
        return new layout::graph::world(d);
    }
    object *o;
    if ((o = mt->cast<object>())) {
        layout::graph::world *w = new layout::graph::world;
        if (w->set(*o, out)) {
            return w;
        }
        w->unref();
    }
    if (out) {
        const char *msg = MPT_tr("unable to get world information");
        if (!name || !*name || len == 0) {
            out->message(_func, out->Warning, "%s", msg);
        } else if (len < 0) {
            out->message(_func, out->Warning, "%s: %s", msg, name);
        } else {
            out->message(_func, out->Warning, "%s: %s", msg, std::string(name, len).c_str());
        }
    }
    return 0;
}
bool layout::graph::bind(const relation &rel, logger *out)
{
    static const char _func[] = "mpt::Graph::bind";
    metatype *mt;
    const char *names, *curr;
    size_t len;

    item_array<axis> oldaxes = _axes;
    _axes = item_array< axis>();

    if (!(names = ::mpt::graph::axes())) {
        for (auto &it : _items) {
            axis *a;
            if (!(mt = it.instance()) || !(a = mt->cast<axis>())) continue;
            curr = it.name();
            if (!a->addref() || add_axis(a, curr)) {
                continue;
            }
            a->unref();
            _axes = oldaxes;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not create axis"), curr ? curr : "");
            return false;
        }
    }
    else while ((curr = mpt_convert_key(&names, 0, &len))) {
        mt = rel.find(typeinfo<::mpt::axis *>::id(), curr, len);
        if (!mt) {
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not find axis"), std::string(curr, len).c_str());
            _axes = oldaxes;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        axis *a = make_axis(mt, out, _func, curr, len);
        if (a && !add_axis(a, curr, len)) {
            a->unref();
            _axes = oldaxes;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign axis"), std::string(curr, len).c_str());
            return false;
        }
    }
    item_array<data> oldworlds = _worlds;
    _worlds = item_array<data>();

    if (!(names = ::mpt::graph::worlds())) {
        for (auto &it : _items) {
            world *w;
            if (!(mt = it.instance()) || !(w = mt->cast<world>())) continue;
            curr = it.name();
            if (!w->addref() || add_world(w, curr)) {
                continue;
            }
            w->unref();
            _axes = oldaxes;
            _worlds = oldworlds;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign world"), curr ? curr : "<>");
            return false;
        }
    }
    else while ((curr = mpt_convert_key(&names, 0, &len))) {
        if (!(mt = rel.find(typeinfo<::mpt::world *>::id(), curr, len))) {
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not find world"), std::string(curr, len).c_str());
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        world *w = make_world(mt, out, _func, curr, len);
        if (w && !add_world(w, curr, len)) {
            w->unref();
            _axes = oldaxes;
            _worlds = oldworlds;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign world"), std::string(curr, len).c_str());
            return false;
        }
    }
    for (auto &it : _items) {
        group *g;
        if (!(mt = it.instance()) || !(g = mt->cast<group>())) continue;
        group_relation gr(*g, &rel);
        if (!g->bind(gr, out)) {
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
    }
    return true;
}
item<layout::graph::axis> *layout::graph::add_axis(axis *from, const char *name, int nlen)
{
    axis *a;

    if (!(a = from)) {
        a = new axis;
    } else {
        for (auto &it : _axes) {
            if (a == it.instance()) return 0; // deny multiple dimensions sharing same transformation
        }
    }
    ::mpt::item<axis> *it;
    if ((it = _axes.append(a, name, nlen))) {
        return it;
    }
    if (!from) a->unref();
    return 0;
}
item<layout::graph::data> *layout::graph::add_world(world *from, const char *name, int nlen)
{
    data *d;
    world *w;

    if (!(w = from)) {
        d = new data(w = new world);
    } else {
        d = new data(w);
    }
    ::mpt::item<data> *it;
    if ((it = _worlds.append(d, name, nlen))) {
        return it;
    }
    d->unref();
    return 0;
}

const reference<cycle> *layout::graph::cycle(int pos) const
{
    static const reference< ::mpt::cycle> def;
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return 0;
    }
    data *d = _worlds.get(pos)->instance();
    if (!d) {
        return 0;
    }
    if (!d->cycle.instance()) {
        class cycle *c = new reference<class cycle>::type;
        d->cycle.set_instance(c);
        world *w;
        if ((w = d->world.instance())) {
            c->limit_dimensions(3);
            c->limit_stages(w->cyc);
        }
    }
    return &d->cycle;
}
bool layout::graph::set_cycle(int pos, const reference<class cycle> &cyc) const
{
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return false;
    }
    data *d = _worlds.get(pos)->instance();
    if (!d) {
        return false;
    }
    d->cycle = cyc;
    return true;
}

const transform &layout::graph::transform()
{
    ::mpt::transform *t;
    if ((t = _gtr.instance())) {
        return *t;
    } else {
        static const class transform def;
        return def;
    }
}

bool layout::graph::update_transform(int dim)
{
    if (dim < 0) {
        update_transform(0);
        update_transform(1);
        update_transform(2);
        return true;
    }
    if (dim > 2) {
        return false;
    }
    ::mpt::item<axis> *it = _axes.get(dim);
    axis *a;

    if (!it || !(a = it->instance())) {
        return false;
    }
    class transform *t;
    if (!(t = _gtr.instance())) {
        t = new class transform;
        _gtr.set_instance(t);
    }
    int type;
    switch (dim) {
    case 0: type = AxisStyleX; break;
    case 1: type = AxisStyleY; break;
    case 2: type = AxisStyleZ; break;
    }
    if (a->::mpt::axis::begin > a->::mpt::axis::end) {
        t->_dim[dim].limit.min = a->::mpt::axis::end;
        t->_dim[dim].limit.max = a->::mpt::axis::begin;
        type |= AxisLimitSwap;
    }
    if (a->format & TransformLg) {
        t->_dim[dim].limit.min = a->::mpt::axis::begin;
        t->_dim[dim].limit.max = a->::mpt::axis::end;
        type |= TransformLg;
    }
    t->_dim[dim].set(t->_dim[dim].limit, type);
    return true;
}
const struct value_apply *layout::graph::transform_part(int dim) const
{
    const class transform *t;
    if (!(t = _gtr.instance())) {
        return 0;
    }
    switch (dim) {
    case 0:
        return &t->_dim[0];
    case 1:
        return &t->_dim[0];
    case 2:
        return &t->_dim[0];
    default:
        return 0;
    }
}
int layout::graph::transform_flags(int dim) const
{
    const class transform *t;
    if (!(t = _gtr.instance())) {
        return 0;
    }
    switch (dim) {
    case 0:
        return t->_dim[0]._flags;
    case 1:
        return t->_dim[0]._flags;
    case 2:
        return t->_dim[0]._flags;
    default:
        return 0;
    }
}

// axis data operations
axis::axis(AxisFlags type)
{
    mpt_axis_init(this);
    format = type & 0x3;
}
axis::axis(const axis & ax)
{
    mpt_axis_init(this, &ax);
}
axis & axis::operator= (const axis & ax)
{
    mpt_axis_fini(this);
    mpt_axis_init(this, &ax);
    return *this;
}
axis::~axis()
{
    mpt_axis_fini(this);
}
layout::graph::axis::axis(const ::mpt::axis *from)
{
    if (!from) return;
    *static_cast<::mpt::axis *>(this) = *from;
}
layout::graph::axis::axis(AxisFlags type) : ::mpt::axis(type)
{ }
layout::graph::axis::~axis()
{ }
void layout::graph::axis::unref()
{
    delete this;
}
int layout::graph::axis::conv(int type, void *ptr) const
{
    int me = typeinfo<axis>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const axis **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            ::mpt::axis::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(metatype::Type)) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return ::mpt::axis::Type;
    }
    if (type == to_pointer_id(::mpt::axis::Type)) {
        if (ptr) *static_cast<const ::mpt::axis **>(ptr) = this;
        return object::Type;
    }
    return BadType;
}
int layout::graph::axis::property(struct property *prop) const
{
    return mpt_axis_get(this, prop);
}
int layout::graph::axis::set_property(const char *prop, const metatype *src)
{
    return mpt_axis_set(this, prop, src);
}
// world data operations
world::world()
{
    mpt_world_init(this);
}
world::world(const world & wld)
{
    mpt_world_init(this, &wld);
}
world & world::operator= (const world & wld)
{
    mpt_world_fini(this);
    mpt_world_init(this, &wld);
    return *this;
}
world::~world()
{
    mpt_world_fini(this);
}
bool world::set_alias(const char *name, int len)
{
    return mpt_string_set(&_alias, name, len) < 0 ? false : true;
}
layout::graph::world::world(const ::mpt::world *from)
{
    if (!from) return;
    *static_cast<::mpt::world *>(this) = *from;
}
layout::graph::world::world(int c)
{
    cyc = (c < 0) ? 1 : c;
}
layout::graph::world::~world()
{ }
void layout::graph::world::unref()
{
    delete this;
}
int layout::graph::world::conv(int type, void *ptr) const
{
    int me = typeinfo<world>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const world **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            ::mpt::world::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(metatype::Type)) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return ::mpt::world::Type;
    }
    if (type == to_pointer_id(::mpt::world::Type)) {
        if (ptr) *static_cast<const ::mpt::world **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<struct color *>(ptr) = color;
        return me;
    }
    if (type == lineattr::Type) {
        if (ptr) *static_cast<lineattr *>(ptr) = attr;
        return me;
    }
    return BadType;
}
int layout::graph::world::property(struct property *prop) const
{
    return mpt_world_get(this, prop);
}
int layout::graph::world::set_property(const char *prop, const metatype *src)
{
    return mpt_world_set(this, prop, src);
}
// graph data operations
layout::graph::data::data(class world *w) : world(w)
{ }
void layout::graph::data::unref()
{
    delete this;
}

__MPT_NAMESPACE_END
