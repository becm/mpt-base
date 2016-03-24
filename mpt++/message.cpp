/*
 * mpt C++ library
 *   message operations
 */

#include "cstdio"
#include "cstring"

#include "convert.h"
#include "message.h"
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

// transport value operations
float80 &float80::swapOrder()
{
    mpt_bswap_80(1, this);
    return *this;
}
float80 & float80::operator= (const long double &val)
{
    mpt_float80_encode(1, &val, this);
    return *this;
}
long double float80::value() const
{
    long double v;
    mpt_float80_decode(1, this, &v);
    return v;
}

// send text message to output
int output::message(const char *from, int type, const char *fmt, ... )
{
    logger *log;
    const char mt[2] = { MessageOutput, (char) type };
    char buf[1024];
    int slen = 0;

    if ((log = mpt_object_logger(this))) {
        va_list ap;
        va_start(ap, fmt);
        slen = log->log(from, type, fmt, ap);
        va_end(ap);
        return slen;
    }
    if (push(sizeof(mt), &mt) < 0) {
        return -1;
    }
    if (from && (slen = push(strlen(from)+1, from)) < 0) {
        slen = 0;
    }
    slen += sizeof(mt);

    if (fmt) {
        va_list ap;
        int alen;
        va_start(ap, fmt);
        alen = vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (alen < 0) {
            alen = snprintf(buf, sizeof(buf), "<%s: %s>", MPT_tr("invalid format string"), fmt);
        }
        if (alen >= (int) sizeof(buf)) {
            alen = sizeof(buf) - 1;
        }
        if (alen && (alen = push(alen, buf)) > 0) {
            slen += alen;
        }
    }
    push(0, 0);

    return slen;
}

__MPT_NAMESPACE_END
