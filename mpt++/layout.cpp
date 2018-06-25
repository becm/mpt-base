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

template class reference_wrapper<layout>;
template class reference_wrapper<layout::line>;
template class reference_wrapper<layout::text>;

template <> int typeinfo<layout>::id()
{
    static int id = 0;
    if (!id && (id = mpt_type_meta_new("layout")) < 0) {
        id = mpt_type_meta_new(0);
    }
    return id;
}

template <> int typeinfo<layout::line>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
}
template <> int typeinfo<layout::text>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_meta_new(0);
    }
    return id;
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

    item_array<graph> arr;

    for (auto &it : _items) {
        metatype *mt;
        graph *g;
        if (!(mt = it.reference()) || !(g = mt->cast<graph>())) {
            continue;
        }
        const char *name = it.name();
        if (!g->addref()) {
            g = new graph();
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
    _graphs = item_array<graph>();
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
        _parse = new config_parser;
        if (!_parse->set_format(file_format())) {
            delete _parse;
            _parse = nullptr;
            return false;
        }
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
        graph *g;
        if (!(g = it.reference())) {
            continue;
        }
        if (g->scale.x < x) x = g->scale.x;
        if (g->scale.y < y) y = g->scale.y;
    }
    return fpoint(x, y);
}

// line data operations
line::line()
{ mpt_line_init(this); }

layout::line::line(const ::mpt::line *from)
{
    if (!from) return;
    *static_cast<::mpt::line *>(this) = *from;
}
layout::line::~line()
{ }
void layout::line::unref()
{
    delete this;
}
int layout::line::conv(int type, void *ptr) const
{
    int me = typeinfo<line>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const line **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            ::mpt::line::Type, color::Type, lineattr::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return ::mpt::line::Type;
    }
    if (type == to_pointer_id(::mpt::line::Type)) {
        if (ptr) *static_cast<const ::mpt::line **>(ptr) = this;
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
int layout::line::property(struct property *prop) const
{
    return mpt_line_get(this, prop);
}
int layout::line::set_property(const char *name, const metatype *src)
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

layout::text::text(const ::mpt::text *from)
{
    if (!from) return;
    *static_cast<::mpt::text *>(this) = *from;
}
layout::text::~text()
{ }
void layout::text::unref()
{
    delete this;
}
int layout::text::conv(int type, void *ptr) const
{
    int me = typeinfo<text>::id();
    if (me < 0) {
        me = metatype::Type;
    }
    else if (type == to_pointer_id(me)) {
        if (ptr) *static_cast<const text **>(ptr) = this;
        return me;
    }
    if (!type) {
        static const uint8_t fmt[] = {
            object::Type,
            ::mpt::text::Type, color::Type, 0
        };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == to_pointer_id(object::Type)) {
        if (ptr) *static_cast<const object **>(ptr) = this;
        return ::mpt::text::Type;
    }
    if (type == to_pointer_id(::mpt::text::Type)) {
        if (ptr) *static_cast<const ::mpt::text **>(ptr) = this;
        return object::Type;
    }
    if (type == color::Type) {
        if (ptr) *static_cast<struct color *>(ptr) = color;
        return me;
    }
    return BadType;
}
int layout::text::property(struct property *prop) const
{
    return mpt_text_get(this, prop);
}
int layout::text::set_property(const char *prop, const metatype *src)
{
    return mpt_text_set(this, prop, src);
}

const char *layout::file_format()
{
    static const char fmt[] = "{*} =;#! '\"\0";
    return fmt;
}

__MPT_NAMESPACE_END
