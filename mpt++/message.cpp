/*
 * mpt C++ library
 *   message operations
 */

#include <stdio.h>

#include "convert.h"
#include "message.h"
#include "output.h"
#include "event.h"

__MPT_NAMESPACE_BEGIN

// message processing
size_t message::read(size_t len, void *base)
{ return mpt_message_read(this, len, base); }
size_t message::length() const
{ return mpt_message_length(this); }

// send text message to output
int output::message(const char *from, int type, const char *fmt, ... )
{
    va_list ap;

    if (fmt) va_start(ap, fmt);
    int ret = mpt_output_vlog(this, from, type, fmt, ap);
    if (fmt) va_end(ap);
    return ret;
}

__MPT_NAMESPACE_END
