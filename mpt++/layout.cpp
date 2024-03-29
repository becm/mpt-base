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

template class reference<layout>;
template class reference<layout::line>;
template class reference<layout::text>;

/*!
 * \ingroup mptLayout
 * \brief get layout interface traits
 * 
 * Get named traits for layout pointer data.
 * 
 * \param obtain  trigger type registration
 * 
 * \return named traits for layout pointer
 */
const struct named_traits *layout::pointer_traits(bool obtain)
{
	static const struct named_traits *traits = 0;
	if (!traits && obtain && !(traits = type_traits::add_metatype("mpt.layout"))) {
		traits = type_traits::add_metatype();
	}
	return traits;
}

// layout extension
layout::layout() : _parse(0), _alias(0), _font(0)
{ }
layout::~layout()
{
	mpt_string_set(&_alias, 0, 0);
	delete _parse;
}
// metatype interface
int layout::convert(type_t type, void *ptr)
{
	if (assign(static_cast<object *>(this), type, ptr)) {
		int type = type_properties<group *>::id(true);
		return type < 0 ? TypeMetaPtr : type;
	}
	return item_group::convert(type, ptr);
}
// object interface
int layout::property(struct property *pr) const
{
	if (!pr) {
		return type_properties<layout *>::id(true);
	}
	const char *name = pr->name;
	int pos = -1;
	if (!name) {
		pos = (uintptr_t) pr->desc;
	}
	else if (!*name) {
		pr->name = "layout";
		pr->desc = "mpt layout data";
		pr->set(_alias);
		return 0;
	}
	int id = 0;
	if (name ? (!strcasecmp(name, "alias") || !strcasecmp(name, "name")) : pos == id++) {
		pr->name = "alias";
		pr->desc = "layout alias";
		pr->set(_alias);
		
		return _alias ? strlen(_alias) : 0;
	}
	if (name ? !strcasecmp(name, "font") : pos == id++) {
		pr->name = "font";
		pr->desc = "layout default font";
		pr->set(_font);
		
		return _font ? strlen(_font) : 0;
	}
	return BadArgument;
}
int layout::set_property(const char *name, convertable *src)
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
int layout::bind(const relation *rel, logger *out)
{
	item_array<metatype> old = _items;
	int ret = 0;
	
	if (!rel) {
		collection::relation me(*this);
		ret = item_group::bind(&me, out);
	} else {
		ret = item_group::bind(rel, out);
	}
	if (ret < 0) {
		return ret;
	}
	item_array<graph> arr;
	
	for (auto &it : _items) {
		metatype *mt;
		graph *g;
		if (!(mt = it.instance()) || !(g = *mt)) {
			continue;
		}
		const char *name = it.name();
		if (!g->addref()) {
			static const char _func[] = "mpt::layout::bind\0";
			::mpt::graph *d;
			object *o = 0;
			if (!(d = *mt) || !(o = *mt)) {
				const char *msg = MPT_tr("unable to get graph information");
				if (out) {
					if (!name || !*name) {
						out->message(_func, out->Warning, "%s", msg);
					} else {
						out->message(_func, out->Warning, "%s: %s", msg, name);
					}
				}
				continue;
			}
			g = new graph(d);
			
			if (o && !g->set(*o, out)) {
				g->unref();
				continue;
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

bool layout::load(logger *out)
{
	static const char _func[] = "mpt::layout::load\0";
	if (!_parse) {
		if (out) {
			out->message(_func, out->Warning, "%s",
			             MPT_tr("no input for layout"));
		}
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
	collection::relation self(*this);
	clear();
	// add items to layout
	if (!add_items(*this, conf, &self, out)) {
		return false;
	}
	// create graphic representations
	if (bind(0, out) < 0) {
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
		if (!(g = it.instance())) {
			continue;
		}
		if (g->scale.x < x) x = g->scale.x;
		if (g->scale.y < y) y = g->scale.y;
	}
	return fpoint(x, y);
}

// line data operations
line::line()
{
	mpt_line_init(this);
}
layout::line::line(const ::mpt::line *from)
{
	if (!from) {
		return;
	}
	*static_cast< ::mpt::line *>(this) = *from;
}
layout::line::~line()
{ }
int layout::line::convert(type_t type, void *ptr)
{
	int me = type_properties<line *>::id(true);
	if (me < 0) {
		me = TypeMetaPtr;
	}
	else if (assign(this, type, ptr)) {
		return me;
	}
	
	if (!type) {
		static const uint8_t fmt[] = {
			TypeObjectPtr,
			0
		};
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		int li = type_properties<::mpt::line>::id(true);
		return li > 0 ? li : me;
	}
	if (assign(static_cast<object *>(this), type, ptr)) {
		int li = type_properties<::mpt::line>::id(true);
		return li > 0 ? li : me;
	}
	if (assign(static_cast< ::mpt::line>(*this), type, ptr)) {
		return TypeObjectPtr;
	}
	if (assign(&color, type, ptr)) {
		int la = type_properties<::mpt::lineattr>::id(true);
		return la > 0 ? la : me;
	}
	if (assign(&attr, type, ptr)) {
		int col = type_properties<::mpt::color>::id(true);
		return col > 0 ? col : me;
	}
	return BadType;
}
void layout::line::unref()
{
	delete this;
}
layout::line *layout::line::clone() const
{
	return new line(this);
}
int layout::line::property(struct property *prop) const
{
	return mpt_line_get(this, prop);
}
int layout::line::set_property(const char *name, convertable *src)
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
{
	return mpt_string_set(&_value, v);
}
bool text::set_font(const char *v)
{
	return mpt_string_set(&_font, v);
}
int text::set(metatype &src)
{
	return mpt_string_pset(&_value, &src);
}
layout::text::text(const ::mpt::text *from)
{
	if (!from) {
		return;
	}
	*static_cast< ::mpt::text *>(this) = *from;
}
layout::text::~text()
{ }
int layout::text::convert(type_t type, void *ptr)
{
	int me = type_properties<text *>::id(true);
	if (me < 0) {
		me = TypeMetaPtr;
	}
	else if (assign(this, type, ptr)) {
		return me;
	}
	if (!type) {
		static const uint8_t fmt[] = {
			TypeObjectPtr,
			0
		};
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		int tx = type_properties<::mpt::text *>::id(true);
		return tx > 0 ? tx : me;
	}
	if (assign(static_cast<object *>(this), type, ptr)) {
		int tx = type_properties<::mpt::text *>::id(true);
		return tx > 0 ? tx : me;
	}
	if (assign(static_cast< ::mpt::text *>(this), type, ptr)) {
		return TypeObjectPtr;
	}
	if (assign(color, type, ptr)) {
		return me;
	}
	return MPT_ERROR(BadType);
}
void layout::text::unref()
{
	delete this;
}
layout::text *layout::text::clone() const
{
	return new text(this);
}
int layout::text::property(struct property *prop) const
{
	return mpt_text_get(this, prop);
}
int layout::text::set_property(const char *prop, convertable *src)
{
	return mpt_text_set(this, prop, src);
}

const char *layout::file_format()
{
	static const char fmt[] = "{*} =;#! '\"\0";
	return fmt;
}

__MPT_NAMESPACE_END
