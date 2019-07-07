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

template <> int typeinfo<client>::id()
{
	return mpt_client_typeid();
}
static void unrefConfig()
{
	if (cfg) {
		cfg->unref();
		cfg = 0;
	}
}
static config *clientConfig()
{
	if (!cfg) {
		static const char dest[] = "mpt.client\0";
		path p;
		p.set(dest);
		if (!(cfg = config::global(&p))) {
			return 0;
		}
		atexit(unrefConfig);
	}
	return cfg->cast<config>();
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
int client::convert(int type, void *ptr)
{
	int me = typeinfo<client>::id();
	if (me < 0) {
		me = metatype::Type;
	}
	else if (type == to_pointer_id(me)) {
		if (ptr) *static_cast<client **>(ptr) = this;
		return config::Type;
	}
	if (!type) {
		static const uint8_t fmt[] = { config::Type, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	if (type == to_pointer_id(config::Type)) {
		if (ptr) *static_cast<class config **>(ptr) = clientConfig();
		return me;
	}
	if (type == to_pointer_id(metatype::Type)) {
		if (ptr) *static_cast<metatype **>(ptr) = this;
		return config::Type;
	}
	return BadType;
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
	 && mt.cmd != msgtype::Command) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadType), "%s (%02x)", MPT_tr("bad message type"), mt.cmd);
		ev->id = 0;
		return ev->Fail | ev->Default;
	}
	int ret = mpt_client_command(this, &msg, mt.arg);
	if (ret < 0) {
		mpt_context_reply(ev->reply, ret, "%s", MPT_tr("command message dispatch failed"));
		ev->id = 0;
		return ev->Fail | ev->Default;
	}
	if (ev->reply) {
		mpt_context_reply(ev->reply, 0, "%s", MPT_tr("command message dispatched"));
	}
	return ev->None;
}

__MPT_NAMESPACE_END
