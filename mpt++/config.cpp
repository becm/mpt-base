/*
 * MPT C++ config interface
 */

#include <sys/uio.h>

#include "node.h"

#include "config.h"

__MPT_NAMESPACE_BEGIN

template class type_properties<config::root>;
template class type_properties<config::item>;

// non-trivial path operations
array::content *path::array_content() const
{
	if (!base || !(flags & HasArray)) {
		return 0;
	}
	return reinterpret_cast<array::content *>(const_cast<char *>(base)) - 1;
}
path::path(const char *path, int s, int a) : base(0), off(0), len(0)
{
	sep = s;
	assign = a;
	mpt_path_set(this, path, -1);
}
path::path(const path &from) : base(0)
{
	*this = from;
}
path::~path()
{
	mpt_path_fini(this);
}
path &path::operator =(const path &from)
{
	if (this == &from) {
		return *this;
	}
	array::content *s = from.array_content(), *t = array_content();
	if (s != t) {
		if (s) s->addref();
		if (t) t->unref();
	}
	memcpy(this, &from, sizeof(*this));
	return *this;
}

span<const char> path::data() const
{
	array::content *d = array_content();
	size_t skip, max;
	if (!d || (skip = off + len) > (max = d->length())) {
		return span<const char>(0, 0);
	}
	return span<const char>(static_cast<char *>(d->data()) + skip, max - skip);
}
bool path::clear_data()
{
	array::content *d = array_content();
	return d ? d->set_length(off + len) : true;
}

void path::set(const char *path, int len, int s, int a)
{
	if (s >= 0) this->sep = s;
	if (a >= 0) this->assign = a;
	mpt_path_set(this, path, len);
}
int path::del()
{
	return mpt_path_del(this);
}
int path::add(int)
{
	return mpt_path_add(this, len);
}
bool path::next()
{
	return (mpt_path_next(this) < 0) ? false : true;
}

/*!
 * \ingroup mptConfig
 * \brief get config interface traits
 * 
 * Get named traits for config pointer data.
 * 
 * \return named traits for config pointer
 */
const struct named_traits *config::pointer_traits()
{
	return mpt_interface_traits(TypeConfigPtr);
}

// default implementation for config
bool config::set(const char *p, const char *val, int sep)
{
	path where(p, sep, 0);
	
	if (!val) {
		return (remove(&where) < 0) ? false : true;
	}
	value tmp(val);
	return assign(&where, &tmp) < 0 ? false : true;
}
convertable *config::get(const char *base, int sep, int len) const
{
	path to;
	to.set(base, len, sep, 0);
	return query(&to);
}
void config::del(const char *p, int sep, int len)
{
	path where;
	where.set(p, len, sep, 0);
	remove(&where);
}
int config::environ(const char *glob, int sep, char * const env[])
{
	return mpt_config_environ(this, glob, sep, env);
}
metatype *config::global(const path *p)
{
	return mpt_config_global(p);
}

// config with item store
config::root::root()
{ }
config::root::~root()
{ }
// config interface
int config::root::assign(const path *dest, const value *val)
{
	// no 'self' or 'root' element(s)
	if (!dest || dest->empty()) {
		return BadArgument;
	}
	// find existing
	path p = *dest;
	metatype *m;
	item *curr;
	
	if (!val) {
		if (!(curr = ::mpt::query(_sub, p))) {
			return 0;
		}
		int type = 0;
		if ((m = curr->instance())) {
			type = m->type();
			curr->set_instance(0);
		}
		return type;
	}
	if (!(m = metatype::create(*val))) {
		return BadType;
	}
	if (!(curr = reserve(_sub, p))) {
		m->unref();
		return BadOperation;
	}
	curr->set_instance(m);
	return m->type();
}
convertable *config::root::query(const path *dest) const
{
	// no 'self' element(s)
	if (!dest || dest->empty()) {
		return 0;
	}
	// find existing
	path p = *dest;
	item *curr;
	
	if (!(curr = ::mpt::query(_sub, p))) {
		return 0;
	}
	return curr->instance();
}
int config::root::remove(const path *dest)
{
	// clear root element
	if (!dest) {
		return 0;
	}
	// clear configuration
	if (dest->empty()) {
		_sub.resize(0);
		return 0;
	}
	path p = *dest;
	item *curr;
	// requested element not found
	if (!(curr = ::mpt::query(_sub, p))) {
		return BadOperation;
	}
	curr->resize(0); // remove childen from element
	curr->set_name(0); // mark element as unused
	curr->set_instance(0); // remove element data
	
	return 0;
}

// configuration element access
config::item *query(const unique_array<config::item> &arr, path &p)
{
	const span<const char> name = p.value();
	int len;
	
	if ((len = mpt_path_next(&p)) < 0) {
		return 0;
	}
	for (config::item *e = arr.begin(), *to = arr.end(); e < to; ++e) {
		if (e->unused() || !e->equal(name.begin(), len)) {
			continue;
		}
		if (!p.empty()) {
			return query(*e, p);
		}
		return e;
	}
	return 0;
}
config::item *reserve(unique_array<config::item> &arr, path &p)
{
	const span<const char> name = p.value();
	int len;
	
	if ((len = mpt_path_next(&p)) < 0) {
		return 0;
	}
	config::item *unused = 0;
	for (config::item *e = arr.begin(), *to = arr.end(); e < to; ++e) {
		if (e->unused()) {
			if (!unused) unused = e;
			continue;
		}
		if (!e->equal(name.begin(), len)) {
			continue;
		}
		if (!p.empty()) {
			return reserve(*e, p);
		}
		return e;
	}
	if (!unused) {
		if (!(unused = arr.insert(arr.length()))) {
			return 0;
		}
	}
	else {
		unused->resize(0);
		unused->set_instance(0);
	}
	unused->set_name(name.begin(), len);
	
	return p.empty() ? unused : reserve(*unused, p);
}

__MPT_NAMESPACE_END
