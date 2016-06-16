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
parsefmt::parsefmt()
{
    mpt_parse_format(this, 0);
}

parseinput::parseinput() : getc((int (*)(void *)) mpt_getchar_stdio), arg(stdin), line(1)
{ }

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
    static const char _func[] = "mpt::Parse::read";
    int ret;

    if (!_d.src.getc) {
        if (out) out->message(_func, out->Error, "%s", MPT_tr("no parser input"));
        return -1;
    }
    if (!_next) {
        if (out) out->message(_func, out->Error, "%s", MPT_tr("no format specified"));
    }
    if ((ret = mpt_parse_node(_next, _nextCtx, &_d, &to)) < 0 && out) {
        const char *fname;
        if (!(fname = to.data())) fname = "";
        if (ret == BadOperation) {
            out->message(_func, out->Error, "%s: %s %zu", MPT_tr("unable to save element"), MPT_tr("line"), _d.src.line);
        } else {
            out->message(_func, out->Error, "%s (%x): %s %u: %s", MPT_tr("parse error"), _d.curr, MPT_tr("line"), (int) _d.src.line, fname);
        }
    }
    return ret;
}

LayoutParser::LayoutParser() : _fn(0)
{
    _d.name.sect = NameNumCont | NameSpace | NameSpecial;
    _d.name.opt  = NameNumCont;

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

const char *LayoutParser::defaultFormat()
{
    static const char fmt[] = "{*} =;#! '\"\0";
    return fmt;
}

__MPT_NAMESPACE_END
