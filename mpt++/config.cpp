/*
 * MPT C++ config interface
 */

#include <cstdio>

#include <errno.h>

#include <sys/uio.h>

#include "node.h"
#include "array.h"
#include "config.h"
#include "parse.h"

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
    if (s < 0) s = this->sep;
    else this->sep = s;
    if (a < 0) a = this->assign;
    else this->assign = a;
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
    path where;

    where.set(p, -1, sep);

    if (!val) {
        return (remove(&where) < 0) ? false : true;
    }
    where.valid = strlen(val)+1;

    Reference<metatype> *r;
    metatype *m;
    if (!(r = query(&where)) || !(m = *r)) {
        return false;
    }
    return (mpt_meta_set(m, "", 0, val) < 0) ? false : true;
}
metatype *config::get(const char *base, int sep, int len)
{
    path to;
    to.set(base, len, sep, 0);
    Reference<metatype> *r = query(&to);
    return r ? *r : 0;
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
    _root = mpt_node_query(mpt_node_get(0, 0), &p);
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
Reference<metatype> *Config::query(const path *dest)
{
    node *n;

    // get last queried */
    if (!dest) {
        return (Reference<metatype> *) _last;
    }
    // find existing
    if (!dest->valid) {
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
        if (!(n = mpt_node_query(_root, &p))) {
            return 0;
        }
        if (!_root) {
            _root = n;
        }
    }
    // use non-root global
    else if (_root) {
        if (!(n = mpt_node_query(_root->children, &p))) {
            return 0;
        }
        if (!_root->children) {
            _root->children = n;
        }
    }
    // interface to global config
    else {
        if (!(n = mpt_node_query(mpt_node_get(0, 0), &p))) {
            return 0;
        }
    }
    if (p.len) n = mpt_node_get(n->children, &p);
    return (Reference<metatype> *) (_last = n);
}
int Config::remove(const path *dest)
{
    node *n = _root;

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

// parser implementation
parsefmt::parsefmt(const char *fmt)
{
    mpt_parse_format(this, fmt);
}

parse::parse(const char *file)
{
    mpt_parse_init(this);
    if (!file || !(source.arg = fopen(file, "r"))) return;
    source.getc = (int (*)(void *)) mpt_getchar_stdio;
}

Parse::Parse(parse *p) : _parse(p), _next(mpt_parse_format_pre)
{ }
Parse::~Parse()
{ }

bool Parse::setFormat(const char *fmt)
{
    int type;
    ParserFcn n;
    
    if (!_parse) {
        return false;
    }
    parsefmt p = _parse->format;
    if ((type = mpt_parse_format(&p, fmt)) < 0
        || !(n = mpt_parse_next_fcn(type))) {
        return false;
    }
    _next = n;
    _parse->format = p;

    return true;
}

int Parse::read(struct node &to, logger *out)
{
    static const char fname[] = "mpt::Parse::read";
    node *(*save)(node *, const path *, int , int);
    node *curr;
    path where;
    int ret;

    if (!_parse || !_parse->source.getc) {
        if (out) mpt_log(out, fname, LogError, "%s", MPT_tr("no parser input"));
        return -1;
    }

    if (!(curr = to.children)) {
        curr = &to;
        _parse->lastop = ParseSection;
        save = mpt_parse_append;
    }
    else {
        save = (node *(*)(node *, const path *, int , int)) mpt_parse_insert;
    }

    while ((ret = _next(_parse, &where)) > 0) {
        node *tmp;
        if (!(tmp = save(curr, &where, _parse->lastop, ret))) {
            if (out) out->error(fname, "%s: %s %zu", MPT_tr("unable to save element"), MPT_tr("line"), _parse->line);
            return -2;
        }
        else if (!curr) to.children = tmp;

        curr = tmp;

        metatype *m;
        if ((m = curr->meta())) {
            uint32_t line = _parse->line;
            m->set(property("line", "I", &line), 0);
        }
        if (ret & ParseSectEnd) {
            where.del();
        } else {
            where.clearData();
        }
        _parse->lastop = ret;
    }
    if (!ret) return 0;

    if (out) out->error(fname, "%s: %s %zu", MPT_tr("parse error"), MPT_tr("line"), _parse->line);

    return -3;
}
size_t Parse::line() const
{ return _parse ? _parse->line : 0; }

bool Parse::reset()
{
    if (!_parse) return false;
    FILE *file = (FILE *) _parse->source.arg;
    if (file && fseek(file, 0, SEEK_SET) < 0) return false;
    _parse->line = 1;
    return true;
}

__MPT_NAMESPACE_END
