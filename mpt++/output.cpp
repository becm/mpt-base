/*!
 * mpt C++ library
 *   output operations
 */

#include <cstdio>
#include <cstdarg>

#include "output.h"

__MPT_NAMESPACE_BEGIN

int output::open(const char *to)
{
    return mpt_object_pset(this, 0, to, 0);
}
int output::message(const char *from, int type, const char *fmt, ... )
{
    va_list va;
    va_start(va, fmt);
    int ret = mpt_output_vlog(this, from, type, fmt, va);
    va_end(va);
    return ret;
}
histinfo::histinfo() : file(0), state(0), mode(0), ignore(0), lsep(0)
{ }

histinfo::~histinfo()
{
    if (!file
        || file == stdin
        || file == stdout
        || file == stderr) {
        return;
    }
    fclose(file);
}

__MPT_NAMESPACE_END
