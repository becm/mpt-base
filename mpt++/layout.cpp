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
void Line::unref()
{
    delete this;
}
int Line::property(struct property *prop) const
{
    if (!prop) {
        return object::Type;
    }
    return mpt_line_get(this, prop);
}
int Line::setProperty(const char *name, metatype *src)
{
    return mpt_line_set(this, name, src);
}
void *Line::toType(int type)
{
    static const char types[] = { metatype::Type, object::Type, line::Type, color::Type, lineattr::Type, 0 };
    switch (type) {
      case 0: return const_cast<char *>(types);
      case object::Type: return static_cast<object *>(this);
      case line::Type: return static_cast<line *>(this);
      case color::Type: return &color;
      case lineattr::Type: return &attr;
      default: return 0;
    }
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
void Text::unref()
{
    delete this;
}
int Text::property(struct property *prop) const
{
    if (!prop) {
        return object::Type;
    }
    return mpt_text_get(this, prop);
}
int Text::setProperty(const char *prop, metatype *src)
{
    return mpt_text_set(this, prop, src);
}
void *Text::toType(int type)
{
    static const char types[] = { metatype::Type, object::Type, text::Type, color::Type, 0 };
    switch (type) {
      case 0: return const_cast<char *>(types);
      case object::Type: return static_cast<object *>(this);
      case text::Type: return static_cast<text *>(this);
      case color::Type: return &color;
      default: return 0;
    }
}
// axis data operations
axis::axis(AxisFlags type)
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
Axis::Axis(AxisFlags type) : axis(type)
{ }
Axis::~Axis()
{ }
void Axis::unref()
{
    delete this;
}
int Axis::property(struct property *prop) const
{
    if (!prop) {
        return object::Type;
    }
    return mpt_axis_get(this, prop);
}
int Axis::setProperty(const char *prop, metatype *src)
{
    return mpt_axis_set(this, prop, src);
}
void *Axis::toType(int type)
{
    static const char types[] = { metatype::Type, object::Type, axis::Type, 0 };
    switch (type) {
      case 0: return const_cast<char *>(types);
      case object::Type: return static_cast<object *>(this);
      case axis::Type: return static_cast<axis *>(this);
      default: return 0;
    }
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
void World::unref()
{
    delete this;
}
int World::property(struct property *prop) const
{
    if (!prop) {
        return object::Type;
    }
    return mpt_world_get(this, prop);
}
int World::setProperty(const char *prop, metatype *src)
{
    return mpt_world_set(this, prop, src);
}
void *World::toType(int type)
{
    static const char types[] = { metatype::Type, object::Type, world::Type, color::Type, lineattr::Type, 0 };
    switch (type) {
      case 0: return const_cast<char *>(types);
      case object::Type: return static_cast<object *>(this);
      case world::Type: return static_cast<world *>(this);
      case color::Type: return &color;
      case lineattr::Type: return &attr;
      default: return 0;
    }
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
        return Collection::Type;
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
void *Graph::toType(int type)
{
    static const char types[] = { metatype::Type, object::Type, graph::Type, color::Type, 0 };
    switch (type) {
      case 0: return const_cast<char *>(types);
      case object::Type: return static_cast<object *>(this);
      case Group::Type: return static_cast<Group *>(this);
      case graph::Type: return static_cast<graph *>(this);
      case color::Type: return &fg;
      default: return 0;
    }
}
// graph group interface
static Axis *makeAxis(object *o, logger *out, const char *_func, const char *name, int len)
{
    if (o->addref()) {
        return static_cast<Axis *>(o);
    }
    Axis *a = new Axis;
    if (a->setProperties(*o, out)) {
        return a;
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
    a->unref();
    return 0;
}
static World *makeWorld(object *o, logger *out, const char *_func, const char *name, int len)
{
    if (o->addref()) {
        return static_cast<World *>(o);
    }
    World *w = new World;

    if (w->setProperties(*o, out)) {
        return w;
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
    w->unref();
    return 0;
}
bool Graph::bind(const Relation &rel, logger *out)
{
    static const char _func[] = "mpt::Graph::bind";
    object *o;
    const char *names, *curr;
    size_t len;

    ItemArray<Axis> oldaxes = _axes;
    _axes = ItemArray<Axis>();

    if (!(names = graph::axes())) {
        for (auto &it : _items) {
            if (!(o = it.pointer()) || o->type() != Axis::Type) continue;
            curr = it.name();
            Axis *a = makeAxis(o, out, _func, curr, -1);
            if (addAxis(a, curr)) {
                continue;
            }
            a->unref();
            _axes = oldaxes;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not create axis"), curr ? curr : "");
            return false;
        }
    }
    else while ((curr = mpt_convert_key(&names, 0, &len))) {
        o = rel.find(Axis::Type, curr, len);
        if (!o) {
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not find axis"), std::string(curr, len).c_str());
            _axes = oldaxes;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        Axis *a = makeAxis(o, out, _func, curr, len);
        if (!addAxis(a, curr, len)) {
            a->unref();
            _axes = oldaxes;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign axis"), std::string(curr, len).c_str());
            return false;
        }
    }
    ItemArray<Data> oldworlds = _worlds;
    _worlds = ItemArray<Data>();

    if (!(names = graph::worlds())) {
        for (auto &it : _items) {
            if (!(o = it.pointer()) || o->type() != World::Type) continue;
            curr = it.name();
            World *w = makeWorld(o, out, _func, curr, -1);
            if (addWorld(w, curr)) {
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
        if (!(o = rel.find(World::Type, curr, len))) {
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
        World *w = makeWorld(o, out, _func, curr, len);
        if (!addWorld(w, curr, len)) {
            w->unref();
            _axes = oldaxes;
            _worlds = oldworlds;
            if (out) out->message(_func, out->Error, "%s: %s", MPT_tr("could not assign world"), std::string(curr, len).c_str());
            return false;
        }
    }
    for (auto &it : _items) {
        Group *g;
        if (!(o = it.pointer()) || o->property(0) != g->Type) continue;
        g = static_cast<Group *>(o);
        GroupRelation gr(*g, &rel);
        if (!g->bind(gr, out)) {
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
    }
    return true;
}
Item<Axis> *Graph::addAxis(Axis *from, const char *name, int nlen)
{
    Axis *a;

    if (!(a = from)) {
        a = new Axis;
    } else {
        for (auto &it : _axes) {
            if (a == it.pointer()) return 0; // deny multiple dimensions sharing same transformation
        }
    }
    Item<Axis> *it;
    if ((it = _axes.append(a, name, nlen))) {
        return it;
    }
    if (!from) a->unref();
    return 0;
}
Item<Graph::Data> *Graph::addWorld(World *from, const char *name, int nlen)
{
    Data *d;
    World *w;

    if (!(w = from)) {
        d = new Data(w = new World);
    } else {
        d = new Data(w);
    }
    Item<Graph::Data> *it;
    if ((it = _worlds.append(d, name, nlen))) {
        return it;
    }
    d->unref();
    return 0;
}

const Reference<Cycle> *Graph::cycle(int pos) const
{
    static const Reference<Cycle> def;
    if (pos < 0 && (pos += _worlds.length()) < 0) return 0;
    Data *d = _worlds.get(pos)->pointer();
    if (!d) return 0;
    if (!d->cycle.pointer()) {
        Cycle *c = new Reference<Cycle>::instance;
        d->cycle.setPointer(c);
        World *w;
        if ((w = d->world.pointer())) {
            c->limitDimensions(3);
            c->limitStages(w->cyc);
        }
    }
    return &d->cycle;
}
bool Graph::setCycle(int pos, const Reference<Cycle> &cyc) const
{
    if (pos < 0 && (pos += _worlds.length()) < 0) return false;
    Data *d = _worlds.get(pos)->pointer();
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

    if (!it || !(a = it->pointer())) {
        return false;
    }
    cutoff = clip;
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
Graph::Data::Data(World *w) : world(w)
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
        return Collection::Type;
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
bool Layout::bind(const Relation &rel, logger *out)
{
    ItemArray<object> old = _items;
    if (!Collection::bind(rel, out)) return false;

    ItemArray<Graph> arr;

    for (auto &it : _items) {
        object *o;
        Graph *g;
        if (!(o = it.pointer()) || o->type() != Graph::Type) continue;
        g = static_cast<Graph *>(o);
        const char *name = it.name();
        if (!g->addref()) {
            g = new Graph();
            if (!g->setProperties(*o, out)) {
                if (out) {
                    static const char _func[] = "mpt::Layout::bind\0";
                    const char *msg = MPT_tr("unable to get graph information");
                    if (!name || !*name) {
                        out->message(_func, out->Warning, "%s", msg);
                    } else {
                        out->message(_func, out->Warning, "%s: %s", msg, name);
                    }
                }
            }
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
        if (!(g = it.pointer())) continue;
        if (g->scale.x < x) x = g->scale.x;
        if (g->scale.y < y) y = g->scale.y;
    }
    return fpoint(x, y);
}

__MPT_NAMESPACE_END

