/*
 * MPT C++ config interface
 */

#include <sys/uio.h>

#include "node.h"
#include "array.h"
#include "config.h"

__MPT_NAMESPACE_BEGIN

// non-trivial path operations
bool path::setValid()
{ return mpt_path_valid(this) < 0 ? false : true; }

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
        buffer *buf = (buffer *) from.base;
        buf[-1].addref();
    }
    return *this;
}

void path::set(const char *path, int len, int s, int a)
{
    if (s >= 0) this->sep = s;
    if (a >= 0) this->assign = a;
    mpt_path_set(this, path, len);
}

bool path::append(const char *path, int add)
{
    if (add < 0) {
        if (!path) return false;
        add = strlen(path);
    }
    char *dest = (char *) mpt_path_append(this, add);
    if (!dest) return false;
    if (add) {
        if (path) memcpy(dest, path, add);
        else memset(dest, 0, add);
    }
    return true;
}

int path::del()
{
    int l = mpt_path_del(this);
    mpt_path_invalidate(this);
    return l;
}

int path::add()
{ return mpt_path_add(this); }

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
Config::Element *Config::getElement(const Array<Config::Element> &arr, path &p)
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
Config::Element *Config::makeElement(Array<Config::Element> &arr, path &p)
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
        size_t pos = arr.length();
        if (!arr.insert(pos, Element()) || !(unused = arr.get(pos))) {
            return 0;
        }
    }
    else {
        unused->clear();
        metatype *m = unused->pointer();
        if (m) m->assign(0);
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
        if (!(curr = getElement(_sub, p))
            || !(m = curr->pointer())) {
            return 0;
        }
        return m->assign(val);
    }
    if (!(curr = makeElement(_sub, p))) {
        return BadOperation;
    }
    int ret = 0;
    if (!(m = curr->pointer()) || (ret = m->assign(val) < 0)) {
        if (val->fmt) return BadValue;
        size_t len = val->ptr ? strlen((const char *) val->ptr) + 1 : 0;
        if (!(m = metatype::create(len))) {
            return BadOperation;
        }
        if (len && (ret = m->assign(val)) < 0) {
            m->unref();
            return ret;
        }
        curr->setPointer(m);
    }
    return ret;
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
        _sub.clear();
        return 0;
    }
    path p = *dest;
    Element *curr;
    // requested element not found
    if (!(curr = getElement(_sub, p))) {
        return BadOperation;
    }
    // remove childen from element
    curr->clear();

    // mark element as unused
    curr->setName(0);

    // try to reset element
    metatype *m = curr->pointer();
    if (m && m->assign(0)) {
        return 2;
    }
    return 0;
}

__MPT_NAMESPACE_END
