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
parser_format::parser_format()
{
    mpt_parse_format(this, 0);
}

parser_input::parser_input() : getc((int (*)(void *)) mpt_getchar_stdio), arg(stdin), line(1)
{ }

parser::parser() : _fn(0)
{
    _d.src.getc = 0;
    _d.src.arg  = 0;
    
    _next.fcn = 0;
    _next.ctx = 0;
}
parser::~parser()
{
    FILE *file = static_cast<FILE *>(_d.src.arg);
    if (file) fclose(file);
    if (_fn) free(_fn);
}

bool parser::reset()
{
    FILE *file = (FILE *) _d.src.arg;
    if (!file || (_d.src.line && (fseek(file, 0, SEEK_SET) < 0))) return false;
    _d.src.line = 0;
    return true;
}

bool parser::open(const char *fn)
{
    if (!_d.src.arg) {
        if (!fn) return true;
    }
    FILE *old, *f = 0;
    if (fn && !(f = fopen(fn, "r"))) {
        return false;
    }
    old = static_cast<FILE *>(_d.src.arg);
    if (old) fclose(old);
    _d.src.getc = (int (*)(void *)) mpt_getchar_stdio;
    _d.src.arg  = f;
    _d.src.line = 0;
    if (_fn) free(_fn);
    _fn = fn ? strdup(fn) : 0;
    return true;
}

static int saveAppend(void *ctx, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val, int last, int curr)
{
	node *next, **pos = static_cast<node **>(ctx);
	if ((next = mpt_node_append(*pos, p, val, last, curr))) {
		*pos = next;
		return 0;
	}
	return BadOperation;
}

int parser::read(struct node &to, logger *out)
{
    static const char _func[] = "mpt::parser::read";

    if (!_d.src.getc) {
        if (out) out->message(_func, out->Error, "%s", MPT_tr("no parser input"));
        return BadArgument;
    }
    if (!_next.fcn) {
        if (out) out->message(_func, out->Error, "%s", MPT_tr("no format specified"));
        return BadArgument;
    }
    node tmp, *ptr = &tmp;
    _d.prev = parser_context::Section;
    int ret = mpt_parse_config(_next.fcn, _next.ctx, &_d, saveAppend, &ptr);
    /* clear created nodes on error */
    if (ret < 0) {
        mpt_node_clear(&tmp);
        if (!out) {
            return ret;
        }
        const char *fn;
        if (!(fn = file())) {
            if (ret == BadOperation) {
                out->message(_func, out->Error, "%s (%x)", MPT_tr("unable to save element"), _d.curr);
            } else {
                out->message(_func, out->Error, "%s (%x)", MPT_tr("parse error"), _d.curr);
            }
            return ret;
        }
        if (ret == BadOperation) {
            out->message(_func, out->Error, "%s (%x): %s %zu: %s",
                         MPT_tr("unable to save element"), _d.curr,
                         MPT_tr("line"), _d.src.line, fn);
        } else {
            out->message(_func, out->Error, "%s (%x): %s %zu: %s",
                         MPT_tr("parse error"), _d.curr,
                         MPT_tr("line"), _d.src.line, fn);
        }
        return ret;
    }
    mpt_node_clear(&to);
    node *child;
    if ((child = tmp.children)) {
        to.children = child;
        tmp.children = 0;
        do { child->parent = &to; } while ((child = child->next));
    }
    return ret;
}

config_parser::config_parser()
{
    _d.name.sect = _d.name.NumCont | _d.name.Space | _d.name.Special;
    _d.name.opt  = _d.name.NumCont;

    _next.fcn = mpt_parse_next_fcn(mpt_parse_format(&_fmt, 0));
    _next.ctx = &_fmt;
}
bool config_parser::reset()
{
    if (!_d.src.line) return true;
    FILE *f = static_cast<FILE *>(_d.src.arg);
    if (!_fn || !f) return false;
    if (!(f = freopen(_fn, "r", f))) return false;
    _d.src.line = 0;
    return true;
}
bool config_parser::set_format(const char *fmt)
{
    int type;
    input_parser_t n;
    parser_format p;
    
    if ((type = mpt_parse_format(&p, fmt)) < 0
        || !(n = mpt_parse_next_fcn(type))) {
        return false;
    }
    _next.fcn = n;
    _fmt = p;

    return true;
}

__MPT_NAMESPACE_END
