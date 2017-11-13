/*!
 * MPT default client config operations
 */

#include <stdlib.h>

#include <sys/uio.h>

#include "config.h"
#include "message.h"
#include "event.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

static metatype *cfg = 0;

static void unrefConfig()
{
    if (cfg) {
        cfg->unref();
        cfg = 0;
    }
}
static config *clientConfig()
{
    static const char dest[] = "mpt.client\0";
    path p;
    
    p.set(dest);
    if (!cfg) {
        atexit(unrefConfig);
        cfg = config::global(&p);
    }
    config *ptr = 0;
    if (cfg->conv(config::Type, &ptr) < 0) {
        return 0;
    }
    return ptr;
}
/*!
 * \ingroup mptClient
 * \brief convert from client
 * 
 * Get interfaces and data from client
 * 
 * \param type  target type code
 * \param ptr   conversion target address
 * 
 * \return conversion result
 */
int client::conv(int type, void *ptr) const
{
    int me = mpt_client_typeid();
    if (me < 0) {
        me = Type;
    }
    if (!type) {
        static const char fmt[] = { metatype::Type, config::Type, 0 };
    if (ptr) *static_cast<const char **>(ptr) = fmt;
        return me;
    }
    if (type == config::Type) {
        if (ptr) *static_cast<class config **>(ptr) = clientConfig();
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const client **>(ptr) = this;
        return config::Type;
    }
    if (type == Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return config::Type;
    }
    return BadType;
}

class eventLogger : public logger
{
public:
    eventLogger(reply_context * = 0);
    int log(const char *, int , const char *, va_list) __MPT_OVERRIDE;
protected:
    reply_context *_ctx;
};
eventLogger::eventLogger(reply_context *rc) : _ctx(rc)
{ }
int eventLogger::log(const char *from, int type, const char *fmt, va_list va)
{
    if (!_ctx) {
        logger *def = logger::defaultInstance();
        if (!def) {
            return 0;
        }
        return def->log(from, type, fmt, va);
    }
    struct message msg;
    msgtype mt(MessageOutput, type & 0x7f);
    
    msg.base = &mt;
    msg.used = sizeof(mt);
    
    struct iovec data[2];
    if (from) {
        data[0].iov_len = strlen(from) + 1;
    } else {
        data[0].iov_len = 1;
        from = "";
    }
    data[0].iov_base = const_cast<char *>(from);
    msg.cont = data;
    msg.clen = 2;
    
    char buf[reply_context::MaxSize];
    int ret, len = sizeof(buf) - data[0].iov_len - msg.used;
    if (!fmt || (ret = snprintf(buf, len, fmt, va)) < 0) {
        ret = 0;
        msg.clen = 1;
    } else if (ret > len) {
        ret = len;
    }
    data[1].iov_base = buf;
    data[1].iov_len  = len;
    
    return _ctx->reply(&msg);
}
/*!
 * \ingroup mptClient
 * \brief convert from client
 * 
 * Get interfaces and data from client
 * 
 * \param type  target type code
 * \param ptr   conversion target address
 * 
 * \return conversion result
 */
int client::dispatch(event *ev)
{
    if (!ev) {
        return 0;
    }
    if (!ev->msg) {
        return process(0);
    }
    message msg = *ev->msg;
    
    msgtype mt;
    if (msg.read(sizeof(mt), &mt) > 0
        && mt.cmd != MessageCommand) {
        mpt_context_reply(ev->reply, MPT_ERROR(BadType), "%s (%02x)", MPT_tr("bad message type"), mt.cmd);
        ev->id = 0;
        return ev->Fail | ev->Default;
    }
    eventLogger el(ev->reply);
    int ret = mpt_client_command(this, &msg, mt.arg, &el);
    if (ret < 0) {
        ev->id = 0;
        return ev->Fail | ev->Default;
    } else {
        return ev->None;
    }
}

__MPT_NAMESPACE_END
