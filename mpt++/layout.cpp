/*
 * graph item implementation
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

Line::Line(const line *from, uintptr_t ref) : Metatype(ref)
{
    if (!from) return;
    line *l = this; *l = *from;
}
Line::~Line()
{ }

Line *Line::addref()
{ return Metatype::addref() ? this : 0; }

int Line::property(struct property *prop, source *src)
{
    if (!prop && !src) {
        return Type;
    }
    return mpt_line_pget(this, prop, src);
}
void *Line::typecast(int type)
{
    switch (type) {
    case metatype::Type: return static_cast<metatype *>(this);
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
{ return mpt_text_set(&_value, v); }

bool text::setFont(const char *v)
{ return mpt_text_set(&_font, v); }

int text::set(source &src)
{ return mpt_text_pset(&_value, &src); }

Text::Text(const text *from, uintptr_t ref) : Metatype(ref), text(from)
{ }
Text::~Text()
{ }

Text *Text::addref()
{ return Metatype::addref() ? this : 0; }

int Text::property(struct property *prop, source *src)
{
    if (!prop && !src) {
        return Type;
    }
    return mpt_text_pget(this, prop, src);
}
void *Text::typecast(int type)
{
    switch (type) {
    case metatype::Type: return static_cast<metatype *>(this);
    case text::Type: return static_cast<text *>(this);
    case color::Type: return &color;
    default: return 0;
    }
}

// axis data operations
axis::axis(AxisFlag type)
{ mpt_axis_init(this); format = type & 0x3; }

axis::~axis()
{ mpt_axis_fini(this); }

Axis::Axis(const axis *from, uintptr_t ref) : Metatype(ref)
{
    if (!from) return;
    axis *ax = this; *ax = *from;
}
Axis::Axis(AxisFlag type, uintptr_t ref) : Metatype(ref), axis(type)
{ }
Axis::~Axis()
{ }

Axis *Axis::addref()
{ return Metatype::addref() ? this : 0; }

int Axis::property(struct property *prop, source *src)
{
    if (!prop && !src) {
        return Type;
    }
    return mpt_axis_pget(this, prop, src);
}
void *Axis::typecast(int type)
{
    switch (type) {
    case Type: return this;
    case metatype::Type: return static_cast<metatype *>(this);
    default: return 0;
    }
}

// world data operations
world::world()
{ mpt_world_init(this); }

world::~world()
{ mpt_world_fini(this); }

bool world::setAlias(const char *name, int len)
{
    return mpt_text_set(&_alias, name, len) < 0 ? false : true;
}

World::World(const world *from, uintptr_t ref) : Metatype(ref)
{
    if (!from) return;
    world *w = this; *w = *from;
}
World::World(int c, uintptr_t ref) : Metatype(ref)
{
    cyc = (c < 0) ? 1 : c;
}

World::~World()
{ }

World *World::addref()
{ return Metatype::addref() ? this : 0; }


int World::property(struct property *prop, source *src)
{
    if (!prop && !src) {
        return Type;
    }
    return mpt_world_pget(this, prop, src);
}
void *World::typecast(int type)
{
    switch (type) {
    case Type: return this;
    case metatype::Type: return static_cast<metatype *>(this);
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

Graph::Graph(const graph *from, uintptr_t ref) : Metatype(ref)
{
    if (!from) return;
    graph *g = this; *g = *from;
}
Graph::~Graph()
{ }

int Graph::unref()
{ return Metatype::unref(); }
Graph *Graph::addref()
{ return Metatype::addref() ? this : 0; }

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
            Axis *a;
            if (!(a = dynamic_cast<Axis *>(m))) {
                continue;
            }
            Reference<Axis> ref(a->addref());
            curr = it.name();
            if ((a = ref) && addAxis(a, curr)) {
                ref.detach();
                continue;
            }
            if (out) out->error(_func, "%s: %s", MPT_tr("could not create axis"), curr ? curr : "");
            _axes = oldaxes;
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
        Axis *a;
        if (!(a = dynamic_cast<Axis *>(m))) {
            if (out) out->error(_func, "%s: %s", MPT_tr("no axis type"), std::string(curr, len).c_str());
            _axes = oldaxes;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        Reference<Axis> ref(a->addref());
        if (!(a = ref) || !addAxis(a, curr, len)) {
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign axis"), std::string(curr, len).c_str());
            _axes = oldaxes;
            return false;
        }
        ref.detach();
    }
    if (!(names = worlds())) {
        for (auto &it : _items) {
            if (!(m = it) || m->type() != World::Type) continue;
            World *w;
            if (!(w = dynamic_cast<World *>(m))) {
                continue;
            }
            Reference<World> ref(w->addref());
            curr = it.name();
            if ((w = ref) && addWorld(w, curr)) {
                ref.detach();
                continue;
            }
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign world"), curr ? curr : "<>");
            _axes = oldaxes;
            _worlds = oldworlds;
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
        World *w;
        if (!(w = dynamic_cast<World *>(m))) {
            if (out) out->error(_func, "%s: %s", MPT_tr("no world type"), std::string(curr, len).c_str());
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
        const char *sep;
        while ((sep = (char *) memchr(curr, ':', len))) {
            len -= (sep - curr) + 1;
            curr = sep + 1;
        }
        Reference<World> ref(w->addref());
        if (!(w = ref) || !addWorld(w, curr, len)) {
            if (out) out->error(_func, "%s: %s", MPT_tr("could not assign world"), std::string(curr, len).c_str());
            _axes = oldaxes;
            _worlds = oldworlds;
            return false;
        }
        ref.detach();
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
bool Graph::set(const struct property &pr, logger *out)
{
    return metatype::set(pr, out);
}

int Graph::property(struct property *prop, source *src)
{
    if (!prop && !src) {
        return Type;
    }
    int ret = mpt_graph_pget(this, prop, src);

    if (ret < 0 || !src) {
        return ret;
    }
    updateTransform();
    return ret;
}
void *Graph::typecast(int type)
{
    switch (type) {
    case metatype::Type: return static_cast<metatype *>(this);
    case graph::Type: return static_cast<graph *>(this);
    case Group::Type: return static_cast<Group *>(this);
    default: return 0;
    }
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
        d->cycle = Reference<Cycle>(new Cycle);
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
Layout::Layout(uintptr_t ref) : Metatype(ref), _parse(0), _alias(0), _font(0)
{ }
Layout::~Layout()
{
    mpt_text_set(&_alias, 0, 0);
    delete _parse;
}

Layout *Layout::addref()
{ return Metatype::addref() ? this : 0; }
int Layout::unref()
{ return Metatype::unref(); }

int Layout::property(struct property *pr, source *src)
{
    const char *name;

    if (!pr) {
        return src ? -1 : Type;
    }
    if (!(name = pr->name)) {
        if (src) return -1;
    }
    if (name ? !*name : !pr->desc) {
        pr->name = "layout";
        pr->desc = "mpt layout data";
        pr->val.fmt = 0;
        pr->val.ptr = _alias;
        return 0;
    }
    if (name ? (!strcasecmp(name, "alias") || !strcasecmp(name, "name")) : ((uintptr_t) pr->desc == 1)) {
        int len;

        if ((len = mpt_text_pset(&_alias, src)) < 0) {
            return -2;
        }
        pr->name = "alias";
        pr->desc = "layout alias";
        pr->val.fmt = 0;
        pr->val.ptr = _alias;

        return len;
    }
    if (name ? !strcasecmp(name, "font") : ((uintptr_t) pr->desc == 2)) {
        int len;

        if ((len = mpt_text_pset(&_font, src)) < 0) {
            return -2;
        }
        pr->name = "font";
        pr->desc = "layout default font";
        pr->val.fmt = 0;
        pr->val.ptr = _font;

        return len;
    }
    return -1;
}
void *Layout::typecast(int t)
{
    switch (t) {
    case metatype::Type: return static_cast<metatype *>(this);
    case Group::Type: return static_cast<Group *>(this);
    default: return 0;
    }
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
        if (!(m = it) || !(g = dynamic_cast<Graph *>(m))) continue;

        Reference<Graph> ref(g->addref());
        if (!(g = ref) || !arr.append(g, it.name())) {
            _items = old;
            return false;
        }
        ref.detach();
    }
    _graphs = arr;
    return true;
}
bool Layout::set(const struct property &pr, logger *log)
{
    return metatype::set(pr, log);
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
    return mpt_text_set(&_alias, base, len) >= 0;
}
bool Layout::setFont(const char *base, int len)
{
    return mpt_text_set(&_font, base, len) >= 0;
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

