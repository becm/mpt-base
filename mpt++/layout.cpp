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

template class reference_wrapper<Line>;
template class reference_wrapper<Text>;
template class reference_wrapper<Axis>;
template class reference_wrapper<World>;
template class reference_wrapper<Graph>;

template class reference_wrapper<layout>;

template <> int typeinfo<Line>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Text>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Axis>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<World>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Graph>::id()
{
    static int id = 0;
    if (!id && (id = mpt_type_meta_new("graph")) < 0) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Graph::data>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<layout>::id()
{
    static int id = 0;
    if (!id && (id = mpt_type_meta_new("layout")) < 0) {
        id = mpt_type_meta_new(0);
    }
    return id;
}

// line data operations
line::line()
{ mpt_line_init(this); }

Line::Line(const line *from)
{
    if (!from) return;
    line *l = this; *l = *from;
}
Line::~Line()
{ }
void Line::unref()
{
    delete this;
}
int Line::conv(int type, void *ptr) const
{
    int me = typeinfo<Line>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const Line **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            line::Type, color::Type, lineattr::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return line::Type;
    }
    if (type == to_pointer_id(line::Type)) {
        if (ptr) *static_cast<const line **>(ptr) = this;
        return object::Type;
    }
    if (type == to_pointer_id(color::Type)) {
        if (ptr) *static_cast<const struct color **>(ptr) = &color;
        return me;
    }
    if (type == to_pointer_id(lineattr::Type)) {
        if (ptr) *static_cast<const lineattr **>(ptr) = &attr;
        return me;
    }
    return BadType;
}
int Line::property(struct property *prop) const
{
    return mpt_line_get(this, prop);
}
int Line::set_property(const char *name, const metatype *src)
{
    return mpt_line_set(this, name, src);
}
// text data operations
text::text()
{
    mpt_text_init(this);
}
text::text(const text &tx)
{
    mpt_text_init(this, &tx);
}
text & text::operator= (const text & tx)
{
    mpt_text_fini(this);
    mpt_text_init(this, &tx);
    return *this;
}
text::~text()
{
    mpt_text_fini(this);
}
bool text::set_value(const char *v)
{ return mpt_string_set(&_value, v); }

bool text::set_font(const char *v)
{ return mpt_string_set(&_font, v); }

int text::set(metatype &src)
{ return mpt_string_pset(&_value, &src); }

Text::Text(const text *from)
{
    if (!from) return;
    *static_cast<text *>(this) = *from;
}
Text::~Text()
{ }
void Text::unref()
{
    delete this;
}
int Text::conv(int type, void *ptr) const
{
    int me = typeinfo<Text>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const Text **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            text::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return text::Type;
    }
    if (type == to_pointer_id(text::Type)) {
        if (ptr) *static_cast<const text **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<struct color *>(ptr) = color;
        return me;
    }
    return BadType;
}
int Text::property(struct property *prop) const
{
    return mpt_text_get(this, prop);
}
int Text::set_property(const char *prop, const metatype *src)
{
    return mpt_text_set(this, prop, src);
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
Axis::Axis(const axis *from)
{
    if (!from) return;
    *static_cast<axis *>(this) = *from;
}
Axis::Axis(AxisFlags type) : axis(type)
{ }
Axis::~Axis()
{ }
void Axis::unref()
{
    delete this;
}
int Axis::conv(int type, void *ptr) const
{
    int me = typeinfo<Axis>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const Axis **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            text::Type, color::Type, 0
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
        return axis::Type;
    }
    if (type == to_pointer_id(axis::Type)) {
        if (ptr) *static_cast<const axis **>(ptr) = this;
        return object::Type;
    }
    return BadType;
}
int Axis::property(struct property *prop) const
{
    return mpt_axis_get(this, prop);
}
int Axis::set_property(const char *prop, const metatype *src)
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
World::World(const world *from)
{
    if (!from) return;
    *static_cast<world *>(this) = *from;
}
World::World(int c)
{
    cyc = (c < 0) ? 1 : c;
}
World::~World()
{ }
void World::unref()
{
    delete this;
}
int World::conv(int type, void *ptr) const
{
    int me = typeinfo<World>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const World **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            world::Type, color::Type, 0
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
        return world::Type;
    }
    if (type == to_pointer_id(world::Type)) {
        if (ptr) *static_cast<const world **>(ptr) = this;
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
int World::property(struct property *prop) const
{
    return mpt_world_get(this, prop);
}
int World::set_property(const char *prop, const metatype *src)
{
    return mpt_world_set(this, prop, src);
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

Graph::Graph(const graph *from)
{
    if (!from) return;
    *static_cast<graph *>(this) = *from;
}
Graph::~Graph()
{ }
// metatype interface
int Graph::conv(int type, void *ptr) const
{
    int me = typeinfo<Graph>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const Graph **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            graph::Type, color::Type, 0
            
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(graph::Type)) {
        if (ptr) *static_cast<const graph **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<color *>(ptr) = fg;
        return me;
    }
    return collection::conv(type, ptr);
}
// object interface
int Graph::property(struct property *prop) const
{
    if (!prop) {
        return typeinfo<graph>::id();
    }
    return mpt_graph_get(this, prop);
}
int Graph::set_property(const char *prop, const metatype *src)
{
    int ret = mpt_graph_set(this, prop, src);

    if (ret < 0 || !src) {
        return ret;
    }
    update_transform();
    return ret;
}
// graph group interface
static Axis *make_axis(metatype *mt, logger *out, const char *_func, const char *name, int len)
{
    axis *d;
    if ((d = mt->cast<axis>())) {
        return new Axis(d);
    }
    object *o;
    if ((o = mt->cast<object>())) {
        Axis *a = new Axis;
        if (a->set(*o, out)) {
             return a;
        }
        a->unref();
    }
    if (out) {
        const char *msg = MPT_tr("unable to apply axis information");
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
static World *make_world(metatype *mt, logger *out, const char *_func, const char *name, int len)
{
    world *d;
    if ((d = mt->cast<world>())) {
        return new World(d);
    }
    object *o;
    if ((o = mt->cast<object>())) {
        World *w = new World;
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
bool Graph::bind(const relation &rel, logger *out)
{
    static const char _func[] = "mpt::Graph::bind";
    metatype *mt;
    const char *names, *curr;
    size_t len;

    item_array<Axis> oldaxes = _axes;
    _axes = item_array< Axis>();

    if (!(names = graph::axes())) {
        for (auto &it : _items) {
            Axis *a;
            if (!(mt = it.reference()) || !(a = mt->cast<Axis>())) continue;
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
        mt = rel.find(typeinfo<axis *>::id(), curr, len);
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
        Axis *a = make_axis(mt, out, _func, curr, len);
        if (a && !add_axis(a, curr, len)) {
            a->unref();
            _axes = oldaxes;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign axis"), std::string(curr, len).c_str());
            return false;
        }
    }
    item_array<data> oldworlds = _worlds;
    _worlds = item_array<data>();

    if (!(names = graph::worlds())) {
        for (auto &it : _items) {
            World *w;
            if (!(mt = it.reference()) || !(w = mt->cast<World>())) continue;
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
        if (!(mt = rel.find(typeinfo<world *>::id(), curr, len))) {
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
        World *w = make_world(mt, out, _func, curr, len);
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
        if (!(mt = it.reference()) || !(g = mt->cast<group>())) continue;
        group_relation gr(*g, &rel);
        if (!g->bind(gr, out)) {
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
    }
    return true;
}
item<Axis> *Graph::add_axis(Axis *from, const char *name, int nlen)
{
    Axis *a;

    if (!(a = from)) {
        a = new Axis;
    } else {
        for (auto &it : _axes) {
            if (a == it.reference()) return 0; // deny multiple dimensions sharing same transformation
        }
    }
    class item<Axis> *it;
    if ((it = _axes.append(a, name, nlen))) {
        return it;
    }
    if (!from) a->unref();
    return 0;
}
item<Graph::data> *Graph::add_world(World *from, const char *name, int nlen)
{
    data *d;
    World *w;

    if (!(w = from)) {
        d = new data(w = new World);
    } else {
        d = new data(w);
    }
    class item<Graph::data> *it;
    if ((it = _worlds.append(d, name, nlen))) {
        return it;
    }
    d->unref();
    return 0;
}

const reference_wrapper<cycle> *Graph::cycle(int pos) const
{
    static const reference_wrapper<class cycle> def;
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return 0;
    }
    data *d = _worlds.get(pos)->reference();
    if (!d) {
        return 0;
    }
    if (!d->cycle.reference()) {
        class cycle *c = new reference_wrapper<class cycle>::instance;
        d->cycle.set_reference(c);
        World *w;
        if ((w = d->world.reference())) {
            c->limit_dimensions(3);
            c->limit_stages(w->cyc);
        }
    }
    return &d->cycle;
}
bool Graph::set_cycle(int pos, const reference_wrapper<class cycle> &cyc) const
{
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return false;
    }
    data *d = _worlds.get(pos)->reference();
    if (!d) {
        return false;
    }
    d->cycle = cyc;
    return true;
}

const transform &Graph::transform()
{
    ::mpt::transform *t;
    if ((t = _gtr.reference())) {
        return *t;
    } else {
        static const class transform def;
        return def;
    }
}

bool Graph::update_transform(int dim)
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
    class item<Axis> *it = _axes.get(dim);
    Axis *a;

    if (!it || !(a = it->reference())) {
        return false;
    }
    class transform *t;
    if (!(t = _gtr.reference())) {
        t = new class transform;
        _gtr.set_reference(t);
    }
    int type;
    switch (dim) {
    case 0: type = AxisStyleX; break;
    case 1: type = AxisStyleY; break;
    case 2: type = AxisStyleZ; break;
    }
    if (a->axis::begin > a->axis::end) {
        t->_dim[dim].limit.min = a->axis::end;
        t->_dim[dim].limit.max = a->axis::begin;
        type |= AxisLimitSwap;
    }
    if (a->format & TransformLg) {
        t->_dim[dim].limit.min = a->axis::begin;
        t->_dim[dim].limit.max = a->axis::end;
        type |= TransformLg;
    }
    t->_dim[dim].set(t->_dim[dim].limit, type);
    return true;
}
const struct value_apply *Graph::transform_part(int dim) const
{
    const class transform *t;
    if (!(t = _gtr.reference())) {
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
int Graph::transform_flags(int dim) const
{
    const class transform *t;
    if (!(t = _gtr.reference())) {
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
// graph data operations
Graph::data::data(World *w) : world(w)
{ }
void Graph::data::unref()
{
    delete this;
}

// layout extension
layout::layout() : _parse(0), _alias(0), _font(0)
{ }
layout::~layout()
{
    mpt_string_set(&_alias, 0, 0);
    delete _parse;
}
// object interface
int layout::property(struct property *pr) const
{
    if (!pr) {
        return typeinfo<layout>::id();
    }
    const char *name = pr->name;
    int pos = -1;
    if (!name) {
        pos = (uintptr_t) pr->desc;
    }
    else if (!*name) {
        pr->name = "layout";
        pr->desc = "mpt layout data";
        pr->val.fmt = 0;
        pr->val.ptr = _alias;
        return 0;
    }
    int id = 0;
    if (name ? (!strcasecmp(name, "alias") || !strcasecmp(name, "name")) : pos == id++) {
        pr->name = "alias";
        pr->desc = "layout alias";
        pr->val.fmt = 0;
        pr->val.ptr = _alias;

        return _alias ? strlen(_alias) : 0;
    }
    if (name ? !strcasecmp(name, "font") : pos == id++) {
        pr->name = "font";
        pr->desc = "layout default font";
        pr->val.fmt = 0;
        pr->val.ptr = _font;

        return _font ? strlen(_font) : 0;
    }
    return BadArgument;
}
int layout::set_property(const char *name, const metatype *src)
{
    int len;

    /* auto-select matching property */
    if (!name) {
        if (!src) {
            return MPT_ERROR(BadOperation);
        }
        if ((len = mpt_string_pset(&_alias, src)) >= 0) {
            return len;
        }
        return MPT_ERROR(BadType);
    }
    /* copy from sibling */
    if (!*name) {
        return MPT_ERROR(BadOperation);
    }
    if (!strcasecmp(name, "alias") || !strcasecmp(name, "name")) {
        return mpt_string_pset(&_alias, src);
    }
    if (!strcasecmp(name, "font")) {
        return mpt_string_pset(&_font, src);
    }
    return BadArgument;
}
bool layout::bind(const relation &rel, logger *out)
{
    item_array<metatype> old = _items;
    if (!collection::bind(rel, out)) {
        return false;
    }

    item_array<Graph> arr;

    for (auto &it : _items) {
        metatype *mt;
        Graph *g;
        if (!(mt = it.reference()) || !(g = mt->cast<Graph>())) {
            continue;
        }
        const char *name = it.name();
        if (!g->addref()) {
            g = new Graph();
            object *o;
            if (!(o = mt->cast<object>())) {
                continue;
            }
            if (g->set(*o, out)) {
                continue;
            }
            if (!out) {
                continue;
            }
            static const char _func[] = "mpt::Layout::bind\0";
            const char *msg = MPT_tr("unable to get graph information");
            if (!name || !*name) {
                out->message(_func, out->Warning, "%s", msg);
                continue;
            }
            out->message(_func, out->Warning, "%s: %s", msg, name);
        }
        if (!arr.append(g, name)) {
            g->unref();
            _items = old;
            return false;
        }
    }
    _graphs = arr;
    return true;
}

bool layout::load(logger *out)
{
    static const char _func[] = "mpt::Layout::load\0";
    if (!_parse) {
        if (out) out->message(_func, out->Warning, "%s", MPT_tr("no input for layout"));
        return false;
    }

    node root;
    if (_parse->read(root, out) < 0) {
        return false;
    }

    // find layout name
    node *conf;

    if (!(conf = root.children)) {
        if (out) {
            const char *n = alias();
            out->message(_func, out->Error, n ? "%s" : "%s: '%s'",
                         MPT_tr("empty layout"), n);
        }
    }
    // add items
    group_relation self(*this);
    clear();
    // add items to layout
    if (!add_items(conf, &self, out)) {
        return false;
    }
    // create graphic representations
    if (!bind(self, out)) {
        return false;
    }
    return true;
}
bool layout::reset()
{
    if (_parse && !_parse->reset()) {
        return false;
    }
    _graphs = item_array<Graph>();
    set_font(0);
    set_alias(0);
    return true;
}

bool layout::open(const char *fn)
{
    if (!_parse) {
        if (!fn) {
            return true;
        }
        _parse = new LayoutParser();
    }
    return _parse->open(fn);
}

bool layout::set_alias(const char *base, int len)
{
    return mpt_string_set(&_alias, base, len) >= 0;
}
bool layout::set_font(const char *base, int len)
{
    return mpt_string_set(&_font, base, len) >= 0;
}

fpoint layout::minimal_scale() const
{
    float x = 1, y = 1;

    for (auto &it : _graphs) {
        Graph *g;
        if (!(g = it.reference())) {
            continue;
        }
        if (g->scale.x < x) x = g->scale.x;
        if (g->scale.y < y) y = g->scale.y;
    }
    return fpoint(x, y);
}

__MPT_NAMESPACE_END
