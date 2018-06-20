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

template <> int typeinfo<Line *>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Text *>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Axis *>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
template <> int typeinfo<World *>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Graph *>::id()
{
    static int id = 0;
    if (!id && (id = mpt_valtype_meta_new("graph")) < 0) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
}
template <> int typeinfo<Graph::Data *>::id()
{
    static int id = 0;
    if (!id) {
        id = make_id();
    }
    return id;
}
template <> int typeinfo<Layout *>::id()
{
    static int id = 0;
    if (!id && (id = mpt_valtype_meta_new("layout")) < 0) {
        id = mpt_valtype_meta_new(0);
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
    int me = typeinfo<Line *>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            metatype::Type, object::Type,
            line::Type, color::Type, lineattr::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == object::Type) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return line::Type;
    }
    if (type == line::Type) {
        if (ptr) *static_cast<const line **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<const struct color **>(ptr) = &color;
        return me;
    }
    if (type == lineattr::Type) {
        if (ptr) *static_cast<const lineattr **>(ptr) = &attr;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const Line **>(ptr) = this;
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
    int me = typeinfo<Text *>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            metatype::Type, object::Type,
            text::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == object::Type) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return text::Type;
    }
    if (type == text::Type) {
        if (ptr) *static_cast<const text **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<const struct color **>(ptr) = &color;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const Text **>(ptr) = this;
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
    int me = typeinfo<Axis *>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            metatype::Type, object::Type,
            text::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == metatype::Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == object::Type) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return axis::Type;
    }
    if (type == axis::Type) {
        if (ptr) *static_cast<const axis **>(ptr) = this;
        return object::Type;
    }
    if (type == me) {
        if (ptr) *static_cast<const Axis **>(ptr) = this;
        return me;
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
    int me = typeinfo<World *>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            metatype::Type, object::Type,
            world::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == metatype::Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return me;
    }
    if (type == object::Type) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return world::Type;
    }
    if (type == world::Type) {
        if (ptr) *static_cast<const world **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<const struct color **>(ptr) = &color;
        return me;
    }
    if (type == lineattr::Type) {
        if (ptr) *static_cast<const lineattr **>(ptr) = &attr;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const World **>(ptr) = this;
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
    int me = typeinfo<Graph *>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            metatype::Type, object::Type,
            graph::Type, color::Type, 0
            
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const Graph **>(ptr) = this;
        return me;
    }
    if (type == graph::Type) {
        if (ptr) *static_cast<const graph **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<const color **>(ptr) = &fg;
        return me;
    }
    return Collection::conv(type, ptr);
}
// object interface
int Graph::property(struct property *prop) const
{
    if (!prop) {
        return typeinfo<Graph *>::id();
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
    axis *d = 0;
    if (mt->conv(axis::Type, &d) >= 0) {
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
    world *d = 0;
    if (mt->conv(world::Type, &d) >= 0) {
        return new World(d);
    }
    object *o;
    if (!(o = mt->cast<object>())) {
        World *w = new World;
        if (w->set(*o, out)) {
            return w;
        }
        w->unref();
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
        mt = rel.find(axis::Type, curr, len);
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
    item_array<Data> oldworlds = _worlds;
    _worlds = item_array<Data>();

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
        if (!(mt = rel.find(world::Type, curr, len))) {
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
item<Graph::Data> *Graph::add_world(World *from, const char *name, int nlen)
{
    Data *d;
    World *w;

    if (!(w = from)) {
        d = new Data(w = new World);
    } else {
        d = new Data(w);
    }
    class item<Graph::Data> *it;
    if ((it = _worlds.append(d, name, nlen))) {
        return it;
    }
    d->unref();
    return 0;
}

const reference_wrapper<Cycle> *Graph::cycle(int pos) const
{
    static const reference_wrapper<Cycle> def;
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return 0;
    }
    Data *d = _worlds.get(pos)->reference();
    if (!d) {
        return 0;
    }
    if (!d->cycle.reference()) {
        Cycle *c = new reference_wrapper<Cycle>::instance;
        d->cycle.set_reference(c);
        World *w;
        if ((w = d->world.reference())) {
            c->limit_dimensions(3);
            c->limit_stages(w->cyc);
        }
    }
    return &d->cycle;
}
bool Graph::set_cycle(int pos, const reference_wrapper<Cycle> &cyc) const
{
    if (pos < 0 && (pos += _worlds.length()) < 0) {
        return false;
    }
    Data *d = _worlds.get(pos)->reference();
    if (!d) {
        return false;
    }
    d->cycle = cyc;
    return true;
}

const Transform &Graph::transform()
{
    Transform *t;
    if ((t = _gtr.reference())) {
        return *t;
    } else {
        static const Transform3 def;
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
    Transform3 *t;
    if (!(t = _gtr.reference())) {
        t = new Transform3;
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
    struct range r(a->axis::begin, a->axis::end);
    t->_dim[dim].transform.set(r, type);
    return true;
}
const struct transform *Graph::transform_part(int dim) const
{
    Transform3 *t;
    if (!(t = _gtr.reference())) {
        return 0;
    }
    switch (dim) {
    case 0:
        return &t->_dim[0].transform;
    case 1:
        return &t->_dim[0].transform;
    case 2:
        return &t->_dim[0].transform;
    default:
        return 0;
    }
}
int Graph::transform_flags(int dim) const
{
    Transform3 *t;
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
Graph::Data::Data(World *w) : world(w)
{ }
void Graph::Data::unref()
{
    delete this;
}

// layout extension
Layout::Layout() : _parse(0), _alias(0), _font(0)
{ }
Layout::~Layout()
{
    mpt_string_set(&_alias, 0, 0);
    delete _parse;
}
// object interface
int Layout::property(struct property *pr) const
{
    if (!pr) {
        return typeinfo<Layout *>::id();
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
int Layout::set_property(const char *name, const metatype *src)
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
bool Layout::bind(const relation &rel, logger *out)
{
    item_array<metatype> old = _items;
    if (!Collection::bind(rel, out)) {
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

bool Layout::load(logger *out)
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
bool Layout::reset()
{
    if (_parse && !_parse->reset()) {
        return false;
    }
    _graphs = item_array<Graph>();
    set_font(0);
    set_alias(0);
    return true;
}

bool Layout::open(const char *fn)
{
    if (!_parse) {
        if (!fn) {
            return true;
        }
        _parse = new LayoutParser();
    }
    return _parse->open(fn);
}

bool Layout::set_alias(const char *base, int len)
{
    return mpt_string_set(&_alias, base, len) >= 0;
}
bool Layout::set_font(const char *base, int len)
{
    return mpt_string_set(&_font, base, len) >= 0;
}

fpoint Layout::minimal_scale() const
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
