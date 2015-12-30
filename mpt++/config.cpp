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
    Reference<metatype> *r;
    metatype *m;
    if (!(r = query(&where, strlen(val)+1)) || !(m = *r)) {
        return false;
    }
    value tmp(0, val);
    return (m->assign(&tmp) < 0) ? false : true;
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
void config::del(const char *p, int sep, int assign)
{
    path where;
    where.set(p, sep, assign);
    remove(&where);
}
int config::environ(const char *glob, int sep, char * const env[])
{ return mpt_config_environ(this, glob, sep, env); }

// config with private or global node store
Config::Config(const char *top, int sep) : _last(0), _root(0), _local(true)
{
    if (!top) return;
    _local = false;
    if (!*top) return;
    path p(sep, 0, top);
    _root = mpt_node_query(mpt_node_get(0, 0), &p, 0);
    if (p.len) _root = mpt_node_get(_root->children, &p);
}
Config::~Config()
{
    if (_local && _root) {
        node top;
        top.children = _root;
        mpt_node_clear(&top);
    }
}
int Config::unref()
{
    delete this;
    return 0;
}
Reference<metatype> *Config::query(const path *dest, int minlen)
{
    node *n;

    // metatype identity */
    if (!dest) {
        if (_local || !_root) {
            _last = 0;
            return 0;
        }
        _last = _root;
        return (Reference<metatype> *) &_root->_meta;
    }
    // find existing
    if (minlen < 0) {
        if (_local) {
            n = _root ? mpt_node_get(_root, dest) : 0;
        }
        else if ((n = _root->children)) {
            n = mpt_node_get(n, dest);
        }
        return n ? (Reference<metatype> *) (_last = n) : 0;
    }
    path p = *dest;

    // query local config
    if (_local) {
        if (!(n = mpt_node_query(_root, &p, minlen))) {
            return 0;
        }
        if (!_root) {
            _root = n;
        }
    }
    // use non-root global
    else if (_root) {
        if (!(n = mpt_node_query(_root->children, &p, minlen))) {
            return 0;
        }
        if (!_root->children) {
            _root->children = n;
            n->parent = _root;
        }
    }
    // interface to global config
    else {
        if (!(n = mpt_node_query(mpt_node_get(0, 0), &p, minlen))) {
            return 0;
        }
    }
    if (p.len) n = mpt_node_get(n->children, &p);
    return (Reference<metatype> *) (_last = n);
}
int Config::remove(const path *dest)
{
    node *n;

    // local search only
    if (_local) {
        n = _root ? mpt_node_get(_root, dest): 0;
    }
    // global search in subtree
    else if ((n = _root)) {
        if ((n = n->children)) {
            n = mpt_node_get(n, dest);
        }
    }
    // general global search
    else {
        n = mpt_node_get(0, dest);
    }
    if (!n) {
        return -2;
    }
    if (n->children) {
        n->setMeta(0);
        _last = n;
        return 0;
    }
    else {
        _last = 0;
        mpt_node_unlink(n);
        mpt_node_destroy(n);
        return 1;
    }
}

__MPT_NAMESPACE_END
