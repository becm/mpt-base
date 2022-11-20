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
template class reference<class layout::graph::transform3>;


template <> int type_properties<lineattr>::id(bool)
{
	return mpt_lattr_typeid();
}

template <> int type_properties<line>::id(bool)
{
	return mpt_line_typeid();
}


template <> int type_properties<graph *>::id(bool)
{
	return mpt_graph_pointer_typeid();
}

template <> int type_properties<axis *>::id(bool)
{
	return mpt_axis_pointer_typeid();
}

template <> int type_properties<text *>::id(bool)
{
	return mpt_axis_pointer_typeid();
}

template <> int type_properties<world *>::id(bool)
{
	return mpt_world_pointer_typeid();
}

template <> int type_properties<layout::graph *>::id(bool obtain)
{
	static const named_traits *traits = 0;
	if (traits) {
		return traits->type;
	}
	if (!obtain) {
		return BadType;
	}
	if ((traits = mpt_type_metatype_add("mpt.graph"))) {
		return traits->type;
	}
	if ((traits = mpt_type_metatype_add(0))) {
		return traits->type;
	}
	return BadOperation;
}
template <> const struct type_traits *type_properties<layout::graph *>::traits() {
	return type_traits::get(id(true));
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
	if (!from) {
		return;
	}
	*static_cast< ::mpt::graph *>(this) = *from;
}
layout::graph::~graph()
{ }
// convertable interface
int layout::graph::convert(int type, void *ptr)
{
	int me = type_properties<graph *>::id(true);
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
	if (assign(static_cast<object *>(this), type, ptr)) {
		int gr = type_properties<::mpt::graph *>::id(true);
		return gr > 0 ? gr : me;
	}
	if (assign(static_cast< ::mpt::graph *>(this), type, ptr)) {
		return TypeObjectPtr;
	}
	if (assign(fg, type, ptr)) {
		return me;
	}
	return item_group::convert(type, ptr);
}
// metatype interface
layout::graph *layout::graph::clone() const
{
	graph *val = new graph(this);
	
	val->_gtr = _gtr;
	val->_axes = _axes;
	val->_worlds = _worlds;
	
	return val;
}
// object interface
int layout::graph::property(struct property *prop) const
{
	if (!prop) {
		return type_properties<graph *>::id(true);
	}
	return mpt_graph_get(this, prop);
}
int layout::graph::set_property(const char *prop, convertable *src)
{
	int ret = mpt_graph_set(this, prop, src);
	
	if (ret < 0 || !src) {
		return ret;
	}
	update_transform();
	return ret;
}
// graph group interface
static layout::graph::axis *make_axis(convertable &from, logger *out, const char *_func, const char *name, int len)
{
	::mpt::axis *d;
	if ((d = from)) {
		return new layout::graph::axis(d);
	}
	object *o;
	if ((o = from)) {
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
static layout::graph::world *make_world(convertable &from, logger *out, const char *_func, const char *name, int len)
{
	::mpt::world *d;
	if ((d = from)) {
		return new layout::graph::world(d);
	}
	object *o;
	if ((o = from)) {
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
int layout::graph::bind(const relation *rel, logger *out)
{
	static const char _func[] = "mpt::layout::graph::bind";
	convertable *from;
	const char *names, *curr;
	size_t len;
	int ret = 0;
	
	item_array<axis> oldaxes = _axes;
	_axes = item_array< axis>();
	
	const collection::relation grel(*this);
	if (!rel) rel = &grel;
	
	if (!(names = ::mpt::graph::axes())) {
		for (auto &it : _items) {
			axis *a;
			if (!(from = it.instance()) || !(a = *from)) {
				continue;
			}
			curr = it.name();
			if (!a->addref()) {
				if (out) {
					out->message(_func, out->Error, "%s: %s",
					             MPT_tr("failed to increase axis reference"), curr ? curr : "");
				}
				continue;
			}
			if (add_axis(a, curr)) {
				++ret;
				continue;
			}
			a->unref();
			_axes = oldaxes;
			return BadOperation;
		}
	}
	else while ((curr = mpt_convert_key(&names, 0, &len))) {
		int id = type_properties< ::mpt::axis *>::id(true);
		if (!(from = rel->find(id, curr, len))) {
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("could not find axis"), std::string(curr, len).c_str());
			}
			_axes = oldaxes;
			return MissingData;
		}
		const char *sep;
		while ((sep = (char *) memchr(curr, ':', len))) {
			len -= (sep - curr) + 1;
			curr = sep + 1;
		}
		axis *a = make_axis(*from, out, _func, curr, len);
		if (a && !add_axis(a, curr, len)) {
			a->unref();
			_axes = oldaxes;
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("could not assign axis"), std::string(curr, len).c_str());
			}
			return BadOperation;
		}
		++ret;
	}
	item_array<data> oldworlds = _worlds;
	_worlds = item_array<data>();
	
	if (!(names = ::mpt::graph::worlds())) {
		for (auto &it : _items) {
			world *w;
			if (!(from = it.instance()) || !(w = *from)) {
				continue;
			}
			curr = it.name();
			if (!w->addref()) {
				if (out) {
					out->message(_func, out->Error, "%s: %s",
					             MPT_tr("failed to increase world reference"), curr ? curr : "");
				}
				continue;
			}
			if (add_world(w, curr)) {
				++ret;
				continue;
			}
			w->unref();
			_axes = oldaxes;
			_worlds = oldworlds;
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("could not assign world"), curr ? curr : "<>");
			}
			return BadOperation;
		}
	}
	else while ((curr = mpt_convert_key(&names, 0, &len))) {
		int id = type_properties< ::mpt::world *>::id(true);
		if (!(from = rel->find(id, curr, len))) {
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("could not find world"), std::string(curr, len).c_str());
			}
			_axes = oldaxes;
			_worlds = oldworlds;
			return MissingData;
		}
		const char *sep;
		while ((sep = (char *) memchr(curr, ':', len))) {
			len -= (sep - curr) + 1;
			curr = sep + 1;
		}
		world *w = make_world(*from, out, _func, curr, len);
		if (w && !add_world(w, curr, len)) {
			w->unref();
			_axes = oldaxes;
			_worlds = oldworlds;
			if (out) {
				out->message(_func, out->Error, "%s: %s",
				             MPT_tr("could not assign world"), std::string(curr, len).c_str());
			}
			return BadOperation;
		}
	}
	for (auto &it : _items) {
		group *g;
		if (!(from = it.instance()) || !(g = *from)) {
			continue;
		}
		collection::relation cr(*g, rel);
		int curr;
		if ((curr = g->bind(&cr, out)) < 0) {
			_axes = oldaxes;
			_worlds = oldworlds;
			return curr;
		}
		ret += curr;
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
			if (a == it.instance()) {
				return 0; // deny multiple dimensions sharing same transformation
			}
		}
	}
	item<axis> *it;
	if ((it = _axes.append(a, name, nlen))) {
		return it;
	}
	if (!from) a->unref();
	return 0;
}
item<layout::graph::data> *layout::graph::add_world(world *from, const char *name, int nlen)
{
	world *w;
	if (!(w = from)) {
		w = new world();
	}
	data *d = new reference<data>::type();
	
	item<data> *it;
	if ((it = _worlds.append(d, name, nlen))) {
		d->world.set_instance(w);
		return it;
	}
	if (!from) {
		w->unref();
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
		static const class transform3 def;
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
	item<axis> *it = _axes.get(dim);
	axis *a;
	
	if (!it || !(a = it->instance())) {
		return false;
	}
	class transform3 *t;
	if (!(t = _gtr.instance())) {
		t = new class transform3;
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
	const class transform3 *t;
	if (!(t = _gtr.instance())) {
		return 0;
	}
	switch (dim) {
		case 0:
			return &t->_dim[0];
		case 1:
			return &t->_dim[1];
		case 2:
			return &t->_dim[2];
		default:
			return 0;
	}
}
int layout::graph::transform_flags(int dim) const
{
	const class transform3 *t;
	if (!(t = _gtr.instance())) {
		return 0;
	}
	switch (dim) {
		case 0:
			return t->_dim[0]._flags;
		case 1:
			return t->_dim[1]._flags;
		case 2:
			return t->_dim[2]._flags;
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
	if (!from) {
		return;
	}
	*static_cast< ::mpt::axis *>(this) = *from;
}
layout::graph::axis::axis(AxisFlags type) : ::mpt::axis(type)
{ }
layout::graph::axis::~axis()
{ }
int layout::graph::axis::convert(int type, void *ptr)
{
	int me = type_properties<axis *>::id(true);
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
		int ax = type_properties<::mpt::axis *>::id(true);
		return ax > 0 ? ax : me;
	}
	if (assign(static_cast<object *>(this), type, ptr)) {
		int ax = type_properties<::mpt::axis *>::id(true);
		return ax > 0 ? ax : me;
	}
	if (assign(static_cast< ::mpt::axis *>(this), type, ptr)) {
		return TypeObjectPtr;
	}
	return BadType;
}
void layout::graph::axis::unref()
{
	delete this;
}
layout::graph::axis *layout::graph::axis::clone() const
{
	return new axis(this);
}
int layout::graph::axis::property(struct property *prop) const
{
	return mpt_axis_get(this, prop);
}
int layout::graph::axis::set_property(const char *prop, convertable *src)
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
	if (!from) {
		return;
	}
	*static_cast< ::mpt::world *>(this) = *from;
}
layout::graph::world::world(int c)
{
	cyc = (c < 0) ? 1 : c;
}
layout::graph::world::~world()
{ }
int layout::graph::world::convert(int type, void *ptr)
{
	int me = type_properties<world *>::id(true);
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
		int wld = type_properties<::mpt::world *>::id(true);
		return wld > 0 ? wld : me;
	}
	if (assign(static_cast<object *>(this), type, ptr)) {
		int wld = type_properties<::mpt::world *>::id(true);
		return wld > 0 ? wld : me;
	}
	if (assign(static_cast< ::mpt::world *>(this), type, ptr)) {
		return TypeObjectPtr;
	}
	if (assign(color, type, ptr)) {
		return me;
	}
	if (assign(attr, type, ptr)) {
		return me;
	}
	return BadType;
}
void layout::graph::world::unref()
{
	delete this;
}
layout::graph::world *layout::graph::world::clone() const
{
	return new world(this);
}
int layout::graph::world::property(struct property *prop) const
{
	return mpt_world_get(this, prop);
}
int layout::graph::world::set_property(const char *prop, convertable *src)
{
	return mpt_world_set(this, prop, src);
}
// graph data operations
layout::graph::data::data(class world *w) : world(w)
{ }

__MPT_NAMESPACE_END
