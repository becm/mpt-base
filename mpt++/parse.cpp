/*
 * MPT C++ config interface
 */

#include <cstdio>

#include <cerrno>
#include <cstring>
#include <cstdlib>

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

Parse::Parse() : _next(0), _nextCtx(0)
{
    _d.src.getc = 0;
    _d.src.arg  = 0;
}
Parse::~Parse()
{
    FILE *file = (FILE *) _d.src.arg;
    if (file) fclose(file);
}

bool Parse::reset()
{
    FILE *file = (FILE *) _d.src.arg;
    if (!file || (_d.src.line && (fseek(file, 0, SEEK_SET) < 0))) return false;
    _d.src.line = 0;
    return true;
}

bool Parse::open(const char *fn)
{
    if (!_d.src.arg) {
        if (!fn) return true;
    }
    FILE *old, *f = 0;
    if (fn && !(f = fopen(fn, "r"))) {
        return false;
    }
    old = (FILE *)_d.src.arg;
    if (old) fclose(old);
    _d.src.getc = (int (*)(void *)) mpt_getchar_stdio;
    _d.src.arg  = f;
    _d.src.line = 0;
    return true;
}

int Parse::read(struct node &to, logger *out)
{
    static const char fname[] = "mpt::Parse::read";
    int ret;

    if (!_d.src.getc) {
        if (out) out->error(fname, "%s", MPT_tr("no parser input"));
        return -1;
    }
    if (!_next) {
        if (out) out->error(fname, "%s", MPT_tr("no format specified"));
    }
    if ((ret = mpt_parse_node(_next, _nextCtx, &_d, &to)) < 0 && out) {
        if (ret == BadOperation) {
            out->error(fname, "%s: %s %zu", MPT_tr("unable to save element"), MPT_tr("line"), _d.src.line);
        } else {
            out->error(fname, "%s (%x): %s %u: %s", MPT_tr("parse error"), _d.curr, MPT_tr("line"), (int) _d.src.line, fname);
        }
    }
    return ret;
}

LayoutParser::LayoutParser() : _fn(0)
{
    _d.check.ctl = (int (*)(void *, const path *, int, int)) checkName;
    _d.check.arg = &_d.name;

    _next = mpt_parse_next_fcn(mpt_parse_format(&_fmt, defaultFormat()));
    _nextCtx = &_fmt;
}
LayoutParser::~LayoutParser()
{
    if (_fn) free(_fn);
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
bool LayoutParser::setFormat(const char *fmt)
{
    int type;
    ParserFcn n;
    parsefmt p;
    
    if (!fmt) fmt = defaultFormat();
    
    if ((type = mpt_parse_format(&p, fmt)) < 0
        || !(n = mpt_parse_next_fcn(type))) {
        return false;
    }
    _next = n;
    _fmt = p;

    return true;
}

int LayoutParser::checkName(const parseflg *flg, const path *p, int , int op)
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
