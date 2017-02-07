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

// message source interface
int MessageSource::reply(const message *)
{
    return -1;
}

// send text message to output
int output::message(const char *from, int type, const char *fmt, ... )
{
    char buf[MPT_OUTPUT_LOGMSG_MAX];
    value val;

    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (len > (int) sizeof(buf)) {
            buf[sizeof(buf) - 1] = 0; // indicate truncation
        }
        val.ptr = buf;
    }
    return log(from, type, &val);
}

__MPT_NAMESPACE_END
