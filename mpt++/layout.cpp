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

template class Reference<Line>;
template class Reference<Text>;
template class Reference<Axis>;
template class Reference<World>;
template class Reference<Graph>;

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
// line object interface
void Line::unref()
{
    delete this;
}
int Line::property(struct property *prop) const
{
    if (!prop) {
        return Type;
    }
    return mpt_line_get(this, prop);
}
int Line::setProperty(const char *name, metatype *src)
{
    return mpt_line_set(this, name, src);
}
// line metatype interface
int Line::assign(const value *val)
{
    return val ? set(0, *val) : setProperty(0, 0);
}
int Line::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;
    
    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, line::Type, color::Type, lineattr::Type, 0 };
        if (dest) *dest = (void *) types;
        return line::Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case line::Type: ptr = static_cast<line *>(this); break;
    case color::Type: ptr = &color; break;
    case lineattr::Type: ptr = &attr; break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *Line::clone()
{
    return new Line(this);
}

// text data operations
text::text(const text *tx)
{ mpt_text_init(this, tx); }
text::~text()
{ mpt_text_fini(this); }
text & text::operator= (const text & tx)
{
    mpt_text_fini(this);
    mpt_text_init(this, &tx);
    return *this;
}

bool text::setValue(const char *v)
{ return mpt_string_set(&_value, v); }

bool text::setFont(const char *v)
{ return mpt_string_set(&_font, v); }

int text::set(metatype &src)
{ return mpt_string_pset(&_value, &src); }

Text::Text(const text *from) : text(from)
{ }
Text::~Text()
{ }
// line object interface
void Text::unref()
{
    delete this;
}
int Text::property(struct property *prop) const
{
    if (!prop) {
        return Type;
    }
    return mpt_text_get(this, prop);
}
int Text::setProperty(const char *prop, metatype *src)
{
    return mpt_text_set(this, prop, src);
}
// text metatype interface
int Text::assign(const struct value *val)
{
    return val ? object::set(0, *val) : setProperty(0, 0);
}
int Text::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;
    
    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, text::Type, color::Type, 0 };
        if (dest) *dest = (void *) types;
        return Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case text::Type: ptr = static_cast<text *>(this); break;
    case color::Type: if (ptr) { memcpy(ptr, &color, sizeof(color)); dest = 0; } break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *Text::clone()
{
    return new Text(this);
}

// axis data operations
axis::axis(AxisFlag type)
{
    mpt_axis_init(this);
    format = type & 0x3;
}
axis::~axis()
{
    mpt_axis_fini(this);
}
Axis::Axis(const axis *from)
{
    if (!from) return;
    axis *ax = this; *ax = *from;
}
Axis::Axis(AxisFlag type) : axis(type)
{ }
Axis::~Axis()
{ }
// axis object interface
void Axis::unref()
{
    delete this;
}
int Axis::property(struct property *prop) const
{
    if (!prop) {
        return Type;
    }
    return mpt_axis_get(this, prop);
}
int Axis::setProperty(const char *prop, metatype *src)
{
    return mpt_axis_set(this, prop, src);
}
// axis metatype interface
int Axis::assign(const value *val)
{
    return val ? set(0, *val) : setProperty(0, 0);
}
int Axis::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;
    
    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, axis::Type, 0 };
        if (dest) *dest = (void *) types;
        return Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case axis::Type: ptr = static_cast<axis *>(this); break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *Axis::clone()
{
    return new Axis(this);
}

// world data operations
world::world()
{
    mpt_world_init(this);
}
world::~world()
{
    mpt_world_fini(this);
}
bool world::setAlias(const char *name, int len)
{
    return mpt_string_set(&_alias, name, len) < 0 ? false : true;
}
World::World(const world *from)
{
    if (!from) return;
    world *w = this; *w = *from;
}
World::World(int c)
{
    cyc = (c < 0) ? 1 : c;
}
World::~World()
{ }
// world object interface
void World::unref()
{
    delete this;
}
int World::property(struct property *prop) const
{
    if (!prop) {
        return Type;
    }
    return mpt_world_get(this, prop);
}
int World::setProperty(const char *prop, metatype *src)
{
    return mpt_world_set(this, prop, src);
}
// world metatype interface
int World::assign(const value *val)
{
    return val ? set(0, *val) : setProperty(0, 0);
}
int World::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;
    
    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, world::Type, color::Type, lineattr::Type, 0 };
        if (dest) *dest = (void *) types;
        return Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case world::Type: ptr = static_cast<world *>(this); break;
    case color::Type: if (ptr) { memcpy(ptr, &color, sizeof(color)); dest = 0; } break;
    case lineattr::Type: if (ptr) { memcpy(ptr, &attr, sizeof(attr)); dest = 0; } break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *World::clone()
{
    return new World(this);
}

// graph data operations
graph::graph()
{ mpt_graph_init(this); }

graph::~graph()
{ mpt_graph_fini(this); }

Graph::Graph(const graph *from)
{
    if (!from) return;
    graph *g = this; *g = *from;
}
Graph::~Graph()
{ }
// graph object interface
void Graph::unref()
{
    delete this;
}
int Graph::property(struct property *prop) const
{
    if (!prop) {
        return Type;
    }
    return mpt_graph_get(this, prop);
}
int Graph::setProperty(const char *prop, metatype *src)
{
    int ret = mpt_graph_set(this, prop, src);

    if (ret < 0 || !src) {
        return ret;
    }
    updateTransform();
    return ret;
}
// graph metatype interface
int Graph::assign(const value *val)
{
    return val ? set(0, *val) : setProperty(0, 0);
}
int Graph::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, graph::Type, color::Type, 0 };
        if (dest) *dest = (void *) types;
        return Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case Group::Type: ptr = static_cast<Group *>(this); break;
    case graph::Type: ptr = static_cast<graph *>(this); break;
    case color::Type: if (ptr) { memcpy(ptr, &fg, sizeof(fg)); dest = 0; } break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *Graph::clone()
{
    return new Graph(this);
}
// graph group interface
static Axis *makeAxis(metatype *m, logger *out, const char *_func, const char *name, int len)
{
    Axis *a;
    if ((a = dynamic_cast<Axis *>(m)) && a->addref()) {
        return a;
    }
    axis *base;

    if (m->conv(axis::Type, &base) < 0) {
        if (out) {
            const char *msg = MPT_tr("unable to get axis information");
            if (!name || !*name || len == 0) {
                out->warning(_func, "%s", msg);
            } else if (len < 0) {
                out->warning(_func, "%s: %s", msg, name);
            } else {
                out->warning(_func, "%s: %s", msg, std::string(name, len).c_str());
            }
        }
        base = 0;
    }
    return new Axis(base);
}
static World *makeWorld(metatype *m, logger *out, const char *_func, const char *name, int len)
{
    World *w;
    if ((w = dynamic_cast<World *>(m)) && w->addref()) {
        return w;
    }
    world *base;

    if (m->conv(world::Type, &base) < 0) {
        if (out) {
            const char *msg = MPT_tr("unable to get axis information");
            if (!name || !*name || len == 0) {
                out->warning(_func, "%s", msg);
            } else if (len < 0) {
                out->warning(_func, "%s: %s", msg, name);
            } else {
                out->warning(_func, "%s: %s", msg, std::string(name, len).c_str());
            }
        }
        base = 0;
    }
    return new World(base);
}
bool Graph::bind(const Relation &rel, logger *out)
{
    static const char _func[] = "mpt::Graph::bind";
    metatype *m;
    const char *names, *curr;
    size_t len;

    ItemArray<Axis> oldaxes = _axes;
    _axes = ItemArray<Axis>();

    RefArray<Data> oldworlds = _worlds;
    _worlds = RefArray<Data>();

    if (!(names = axes())) {
        for (auto &it : _items) {
            if (!(m = it) || m->type() != Axis::Type) continue;
            curr = it.name();
            Axis *a = makeAxis(m, out, _func, curr, -1);
            if (addAxis(a, curr)) {
                continue;
            }
            a->unref();
            _axes = oldaxes;
            if (out) out->error(_func, "%s: %s", MPT_tr("could not create axis"), curr ? curr : "");
            return false;
        }
    }
    else while ((curr = mpt_convert_key(&names, 0, &len))) {
        m = rel.find(Axis::Type, curr, len);
        if (!m) {
            if (out) out->error(_func, "%s: %s", MPT_tr("could not find axis"), std::string(curr, len).c_str());
            _axes = oldaxes;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        Axis *a = makeAxis(m, out, _func, curr, len);
        if (!addAxis(a, curr, len)) {
            a->unref();
            _axes = oldaxes;
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign axis"), std::string(curr, len).c_str());
            return false;
        }
    }
    if (!(names = worlds())) {
        for (auto &it : _items) {
            if (!(m = it) || m->type() != World::Type) continue;
            curr = it.name();
            World *w = makeWorld(m, out, _func, curr, -1);
            if (addWorld(w, curr)) {
                continue;
            }
            w->unref();
            _axes = oldaxes;
            _worlds = oldworlds;
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign world"), curr ? curr : "<>");
            return false;
        }
    }
    else while ((curr = mpt_convert_key(&names, 0, &len))) {
        if (!(m = rel.find(World::Type, curr, len))) {
            if (out) out->error(_func, "%s: %s", MPT_tr("could not find world"), std::string(curr, len).c_str());
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        World *w = makeWorld(m, out, _func, curr, len);
        if (!addWorld(w, curr, len)) {
            w->unref();
            _axes = oldaxes;
            _worlds = oldworlds;
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign world"), std::string(curr, len).c_str());
            return false;
        }
    }
    for (auto &it : _items) {
        Group *g;
        if (!(m = it) || !(g = m->cast<Group>())) continue;
        GroupRelation gr(*g, &rel);
        if (!g->bind(gr, out)) {
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
    }
    return true;
}
const Item<Axis> &Graph::axis(int pos) const
{
    static const Item<Axis> def;
    if (pos < 0 && (pos += _axes.size()) < 0) return def;
    Item<Axis> *a = _axes.get(pos);
    return a ? *a : def;
}
Item<Axis> *Graph::addAxis(Axis *from, const char *name, int nlen)
{
    Axis *a;

    if (!(a = from)) {
        a = new Axis;
    }
    else {
        for (auto &it : _axes) {
            if (a == it) return 0; // deny multiple dimensions sharing same transformation
        }
    }
    Item<Axis> *it;
    if ((it = _axes.append(a, name, nlen))) {
        return it;
    }
    if (!from) a->unref();
    return 0;
}

const Graph::Data &Graph::world(int pos) const
{
    static const Data def;
    if (pos < 0 && (pos += _worlds.size()) < 0) return def;
    Data *d = _worlds.get(pos);
    return d ? *d : def;
}
Graph::Data *Graph::addWorld(World *from, const char *name, int nlen)
{
    Data *d;
    World *w;

    if (!(w = from)) {
        d = new Data(w = new World);
    }
    else {
        d = new Data(w);
    }
    if (d->setName(name, nlen)
        && _worlds.insert(_worlds.size(), d)) {
        return d;
    }
    if (!from) d->Item<World>::detach();
    d->unref();
    return 0;
}

const Reference<Cycle> &Graph::cycle(int pos) const
{
    static const Reference<Cycle> def;
    if (pos < 0 && (pos += _worlds.size()) < 0) return def;
    Data *d = _worlds.get(pos);
    if (!d->cycle) {
        d->cycle = Reference<Cycle>(new Reference<Cycle>::instance);
        World *w;
        if ((w = *d)) {
            static_cast<Cycle *>(d->cycle)->setSize(w->cyc);
        }
    }
    return d->cycle;
}
bool Graph::setCycle(int pos, const Reference<Cycle> &cyc) const
{
    if (pos < 0 && (pos += _worlds.size()) < 0) return false;
    Data *d = _worlds.get(pos);
    if (!d) return false;
    d->cycle = cyc;
    return true;
}

const Transform &Graph::transform()
{ return *this; }

bool Graph::updateTransform(int dim)
{
    if (dim < 0) {
        updateTransform(0);
        updateTransform(1);
        updateTransform(2);
    }
    Item<Axis> *it = _axes.get(dim);
    Axis *a;

    if (!it || !(a = *it)) {
        return false;
    }
    cut = clip;
    switch (dim) {
    case 0:
        fx = tx.fromAxis(*a, AxisStyleX);
        return true;
    case 1:
        fy = ty.fromAxis(*a, AxisStyleY);
        return true;
    case 2:
        fz = tz.fromAxis(*a, AxisStyleZ);
        return true;
    default: return false;
    }
}

// graph data operations
Graph::Data::Data(World *w) : Item<World>(w)
{ }

// layout extension
Layout::Layout() : _parse(0), _alias(0), _font(0)
{ }
Layout::~Layout()
{
    mpt_string_set(&_alias, 0, 0);
    delete _parse;
}
// layout object interface
void Layout::unref()
{
    delete this;
}
int Layout::property(struct property *pr) const
{
    if (!pr) {
        return Type;
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
int Layout::setProperty(const char *name, metatype *src)
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
// layout metatype interface
int Layout::assign(const struct value *val)
{
    return val ? object::set(0, *val) : setProperty(0, 0);
}
int Layout::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { metatype::Type, object::Type, Group::Type, 0 };
        if (dest) *dest = (void *) types;
        return Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case object::Type: ptr = static_cast<object *>(this); break;
    case Group::Type: ptr = static_cast<Group *>(this); break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
}
metatype *Layout::clone()
{
    Layout *lay = new Layout;

    lay->_items  = _items;
    lay->_graphs = _graphs;

    lay->setAlias(_alias);
    lay->setFont(_font);

    return lay;
}

const Item<Graph> &Layout::graph(int pos) const
{
    static const Item<Graph> def;
    if (pos < 0 && (pos += _graphs.size()) < 0) return def;
    Item<Graph> *g;
    return (g = _graphs.get(pos)) ? *g : def;
}

bool Layout::update(metatype *m)
{
    if (!m) return false;
    for (auto &it : _items) {
        metatype *ref = it;
        if (m == ref) return true;
    }
    return false;
}
bool Layout::bind(const Relation &rel, logger *out)
{
    ItemArray<metatype> old = _items;
    if (!Collection::bind(rel, out)) return false;

    ItemArray<Graph> arr;

    for (auto &it : _items) {
        metatype *m;
        Graph *g;
        if (!(m = it) || m->type() != Graph::Type) continue;

        const char *name = it.name();
        if (!(g = dynamic_cast<Graph *>(m)) || !g->addref()) {
            struct graph *base;
            if (m->conv(graph::Type, &base) < 0) {
                if (out) {
                    static const char _func[] = "mpt::Layout::bind\0";
                    const char *msg = MPT_tr("unable to get graph information");
                    if (!name || !*name) {
                        out->warning(_func, "%s", msg);
                    } else {
                        out->warning(_func, "%s: %s", msg, name);
                    }
                }
                base = 0;
            }
            g = new Graph(base);
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
    if (!_parse) {
        if (out) out->warning(__func__, "%s", MPT_tr("no input for layout"));
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
            out->error(__func__, n ? "%s" : "%s: '%s'",
                       MPT_tr("empty layout"), n);
        }
    }
    // add items
    GroupRelation self(*this);
    clear();
    // add items to layout
    if (!addItems(conf, &self, out)) return false;
    // create graphic representations
    if (!bind(self, out)) return false;

    return true;
}
void Layout::reset()
{
    _graphs = ItemArray<Graph>();
    setFont(0);
    setAlias(0);
}

bool Layout::open(const char *fn)
{
    if (!_parse) {
        if (!fn) return true;
        _parse = new LayoutParser();
    }
    return _parse->open(fn);
}

bool Layout::setAlias(const char *base, int len)
{
    return mpt_string_set(&_alias, base, len) >= 0;
}
bool Layout::setFont(const char *base, int len)
{
    return mpt_string_set(&_font, base, len) >= 0;
}

fpoint Layout::minScale() const
{
    float x = 1, y = 1;

    for (auto &it : _graphs) {
        Graph *g;
        if (!(g = it)) continue;
        if (g->scale.x < x) x = g->scale.x;
        if (g->scale.y < y) y = g->scale.y;
    }
    return fpoint(x, y);
}

__MPT_NAMESPACE_END

