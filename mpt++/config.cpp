/*
 * MPT C++ config interface
 */

#include <sys/uio.h>

#include "node.h"
#include "array.h"
#include "config.h"

__MPT_NAMESPACE_BEGIN

// non-trivial path operations
path::path(int s, int a, const char *path) : base(0), off(0), len(0)
{
    sep = s;
    assign = a;
    mpt_path_set(this, path, -1);
}
path::path(const path &from)
{
    *this = from;
}
path::~path()
{
    mpt_path_fini(this);
}
path &path::operator =(const path &from)
{
    memcpy(this, &from, sizeof(*this));
    if (from.flags & HasArray) {
        buffer *buf = reinterpret_cast<buffer *>(const_cast<char *>(from.base));
        buf[-1].addref();
    }
    return *this;
}

Slice<const char> path::data() const
{
    if (flags & HasArray) {
        buffer *buf = (buffer *) base;
        return Slice<const char>(base, buf[-1]._used - off - len);
    }
    return Slice<const char>(0, 0);
}
bool path::clearData()
{
    if (!(flags & HasArray)) {
        return true;
    }
    buffer *buf = (buffer *) base;
    size_t max = off + len;
    if (max > buf[-1]._used) return false;
    buf[-1]._used = max;
    return true;
}

void path::set(const char *path, int len, int s, int a)
{
    if (s >= 0) this->sep = s;
    if (a >= 0) this->assign = a;
    mpt_path_set(this, path, len);
}

int path::del()
{
    int l = mpt_path_del(this);
    mpt_path_invalidate(this);
    return l;
}

int path::add(int)
{ return mpt_path_add(this, len); }

bool path::next()
{ return (mpt_path_next(this) < 0) ? false : true; }


// default implementation for config
bool config::set(const char *p, const char *val, int sep)
{
    path where(sep, 0, p);

    if (!val) {
        return (remove(&where) < 0) ? false : true;
    }
    value tmp(0, val);

    return assign(&where, &tmp) < 0 ? false : true;
}
metatype *config::get(const char *base, int sep, int len)
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
config *config::global(const path *p)
{
	return mpt_config_global(p);
}
// config with private or global node store
Config::Config()
{ }
Config::~Config()
{ }
void Config::unref()
{
    delete this;
}
Config::Element *Config::getElement(const UniqueArray<Config::Element> &arr, path &p)
{
    const Slice<const char> name = p.value();
    int len;

    if ((len = mpt_path_next(&p)) < 0) {
        return 0;
    }
    for (Element *e = arr.begin(), *to = arr.end(); e < to; ++e) {
        if (e->unused() || !e->equal(name.base(), len)) {
            continue;
        }
        if (!p.empty()) {
            return getElement(*e, p);
        }
        return e;
    }
    return 0;
}
Config::Element *Config::makeElement(UniqueArray<Config::Element> &arr, path &p)
{
    const Slice<const char> name = p.value();
    int len;

    if ((len = mpt_path_next(&p)) < 0) {
        return 0;
    }
    Element *unused = 0;
    for (Element *e = arr.begin(), *to = arr.end(); e < to; ++e) {
        if (e->unused()) {
            if (!unused) unused = e;
        }
        if (!e->equal(name.base(), len)) {
            continue;
        }
        if (!p.empty()) {
            return makeElement(*e, p);
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
        unused->setPointer(0);
    }
    unused->setName(name.base(), len);

    return p.empty() ? unused : makeElement(*unused, p);
}
int Config::assign(const path *dest, const value *val)
{
    // no 'self' element(s)
    if (!dest || dest->empty()) {
        return BadArgument;
    }
    // find existing
    path p = *dest;
    metatype *m;
    Element *curr;

    if (!val) {
        if (!(curr = getElement(_sub, p))) {
            return 0;
        }
        int type = 0;
        if ((m = curr->pointer())) type = m->type();
        curr->setPointer(0);
        return type;
    }
    if (!(m = metatype::create(*val))) {
        return BadType;
    }
    if (!(curr = makeElement(_sub, p))) {
        m->unref();
        return BadOperation;
    }
    curr->setPointer(m);
    return m->type();
}
metatype *Config::query(const path *dest) const
{
    // no 'self' element(s)
    if (!dest || dest->empty()) {
        return 0;
    }
    // find existing
    path p = *dest;
    Element *curr;

    if (!(curr = getElement(_sub, p))) {
        return 0;
    }
    return curr->pointer();
}
int Config::remove(const path *dest)
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
    Element *curr;
    // requested element not found
    if (!(curr = getElement(_sub, p))) {
        return BadOperation;
    }
    curr->resize(0); // remove childen from element
    curr->setName(0); // mark element as unused
    curr->setPointer(0); // remove element data

    return 0;
}

__MPT_NAMESPACE_END
