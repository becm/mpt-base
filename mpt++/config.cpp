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
    if (from.flags & PathHasArray) {
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

Slice<const char> path::data() const
{
    return Slice<const char>(mpt_path_data(this), valid);
}

// default implementation for config
bool config::set(const char *p, const char *val, int sep)
{
    path where(sep, 0, p);

    if (!val) {
        return (remove(&where) < 0) ? false : true;
    }
    value tmp(0, val);
    Reference<metatype> *r;
    metatype *m;
    if (!(r = query(&where, &tmp))) {
        return false;
    }
    if (!(m = *r)) {
        if (!(m = mpt_meta_new(strlen(val)+1))) {
            return false;
        }
        if (m->assign(&tmp) < 0) {
            return false;
        }
        r->setReference(m);
    }
    return true;
}
metatype *config::get(const char *base, int sep, int len)
{
    path to;
    to.set(base, len, sep, 0);
    Reference<metatype> *r;
    if (!(r = query(&to))) return 0;
    metatype *m = *r;
    return m;
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
Config::Element *Config::getElement(Array<Config::Element> &arr, path &p, bool require)
{
    const char *name;
    int len;

    name = p.base + p.off;
    if ((len = mpt_path_next(&p)) < 0) {
        return 0;
    }
    Element *unused = 0;
    for (Element *e = arr.begin(), *to = arr.end(); e < to; ++e) {
        if (e->unused()) {
            if (!unused) unused = e;
        }
        if (!e->equal(name, len)) {
            continue;
        }
        if (p.len) {
            return getElement(*e, p, require);
        }
        return e;
    }
    if (!require) {
        return 0;
    }
    if (!unused) {
        size_t pos = arr.size();
        if (!arr.insert(pos, Element()) || !(unused = arr.get(pos))) {
            return 0;
        }
    }
    else {
        unused->clear();
        metatype *m = *unused;
        if (m) m->assign(0);
    }
    unused->setName(name, len);

    return p.len ? getElement(*unused, p, require) : unused;
}
Reference<metatype> *Config::query(const path *dest, const value *val)
{
    // metatype identity */
    if (!dest || !dest->len) {
        if (!val) {
            return this;
        }
        if (_ref && !_ref->assign(val)) {
            return 0;
        }
        return this;
    }
    // find existing
    path p = *dest;
    //const char *name = p.base;
    //int len;
    
    Reference<metatype> *mr = this;
    Element *curr = 0;
    
    if (dest && dest->len) {
        if (!(curr = getElement(_sub, p, val))) {
            return 0;
        }
        mr = curr;
    }
    if (!val) {
        return mr;
    }
    metatype *m;
    if ((m = *mr)) {
        if (m->assign(val)) {
            return curr;
        }
    }
    // TODO: create and assign new metatype
    return mr;
}
int Config::remove(const path *dest)
{
    // clear root element
    if (!dest) {
        if (_ref) _ref->assign(0);
        return 0;
    }
    // clear configuration
    if (!dest->len) {
        _sub.clear();
        if (_ref && _ref->assign(0) < 0) {
            return 1;
        }
        return 0;
    }
    path p = *dest;
    Element *curr;
    // requested element not found
    if (!(curr = getElement(_sub, p, false))) {
        return BadOperation;
    }
    // remove childen from element
    curr->clear();

    // mark element as unused
    curr->setName(0);

    // try to reset element
    metatype *m = *curr;
    if (m && m->assign(0)) {
        return 2;
    }
    return 0;
}

__MPT_NAMESPACE_END
