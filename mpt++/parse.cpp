/*
 * MPT C++ config interface
 */

#include <cstdio>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "node.h"
#include "config.h"
#include "parse.h"

__MPT_NAMESPACE_BEGIN

// parser implementation
parsefmt::parsefmt(const char *fmt)
{
    mpt_parse_format(this, fmt);
}

parseinput::parseinput() : getc((int (*)(void *)) mpt_getchar_stdio), arg(stdin), line(1)
{ }

parse::parse()
{ mpt_parse_init(this); }

Parse::Parse() : _parse(0), _next(mpt_parse_format_pre)
{ }
Parse::~Parse()
{
    if (!_parse) return;
    FILE *file = (FILE *) _parse->src.arg;
    if (file) fclose(file);
    delete _parse;
}
size_t Parse::line() const
{ return _parse ? _parse->src.line : 0; }

bool Parse::reset()
{
    if (!_parse) return false;
    FILE *file = (FILE *) _parse->src.arg;
    if (!file || (_parse->src.line && (fseek(file, 0, SEEK_SET) < 0))) return false;
    _parse->src.line = 0;
    return true;
}

bool Parse::open(const char *fn)
{
    if (!_parse) {
        if (!fn) return true;
        _parse = new parse;
        _parse->src.getc = 0;
    }
    FILE *old, *f = 0;
    if (fn && !(f = fopen(fn, "r"))) {
        return false;
    }
    old = (FILE *)_parse->src.arg;
    if (old) fclose(old);
    _parse->src.getc = (int (*)(void *)) mpt_getchar_stdio;
    _parse->src.arg  = f;
    _parse->src.line = 0;
    return true;
}

bool Parse::setFormat(const char *fmt)
{
    int type;
    ParserFcn n;

    if (!_parse) {
        _parse = new parse();
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

    if (!_parse || !_parse->src.getc) {
        if (out) out->error(fname, "%s", MPT_tr("no parser input"));
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
            if (out) out->error(fname, "%s: %s %zu", MPT_tr("unable to save element"), MPT_tr("line"), _parse->src.line);
            return -2;
        }
        else if (!curr) to.children = tmp;

        curr = tmp;

        metatype *m;
        if ((m = curr->meta())) {
            uint32_t line = _parse->src.line;
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

    if (out) out->error(fname, "%s: %s %zu", MPT_tr("parse error"), MPT_tr("line"), _parse->src.line);

    return -3;
}

LayoutParser::LayoutParser() : _fn(0)
{
    _parse = &_d;

    _d.src.getc = 0;
    _d.src.arg  = 0;

    _d.check.ctl = (int (*)(void *, const path *, int)) checkName;
    _d.check.arg = &_d.name;

    _next = mpt_parse_next_fcn(mpt_parse_format(&_d.format, defaultFormat()));
}
LayoutParser::~LayoutParser()
{
    if (_fn) free(_fn);
    _parse = 0;
}
bool LayoutParser::reset()
{
    if (!_d.src.line) return true;
    if (!_fn) return false;
    return Parse::open(_fn);
}
bool LayoutParser::open(const char *fn)
{
    if (!fn) {
        if (!Parse::open(0)) return false;
        if (_fn) free(_fn);
        return true;
    }
    if (!Parse::open(fn)) {
        return false;
    }
    char *n = strdup(fn);
    if (_fn) free(_fn);
    _fn = n;

    return true;
}

int LayoutParser::checkName(const parseflg *flg, const path *p, int op)
{
    Slice<const char> data = p->data();
    const char *name = data.base();
    size_t len = data.len();
    
    switch (op & 0x3) {
      case MPT_ENUM(ParseSection):
        break;
      case MPT_ENUM(ParseOption):
        return mpt_parse_ncheck(name, len, flg->opt);
      default:
        return 0;
    }
    
    // remove type info
    if (len) {
        while (!isspace(*name)) {
            if (!--len) return -1;
            ++name;
        }
        while (isspace(*name)) {
            if (!--len) return -1;
            ++name;
        }
    }
    if (len) {
        size_t pos = 1;
        
        // get element name end
        while (pos < len && !isspace(name[pos]) && name[pos] != ':') {
           ++pos;
        }
        return mpt_parse_ncheck(name, pos, flg->sect);
    }
    return -2;
}
const char *LayoutParser::defaultFormat()
{
    static const char fmt[] = "{*} =;#! '\"\0";
    return fmt;
}

__MPT_NAMESPACE_END
