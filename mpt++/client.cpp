/*!
 * MPT default client config operations
 */

#include <stdlib.h>

#include "config.h"
#include "message.h"
#include "event.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

static metatype *cfg = 0;

template <> int type_properties<client *>::id(bool obtain)
{
	static const named_traits *traits = 0;
	if (traits) {
		return traits->type;
	}
	if (!obtain) {
		return BadType;
	}
	if ((traits = client::pointer_traits())) {
		return traits->type;
	}
	return BadType;
}
template <> const struct type_traits *type_properties<client *>::traits()
{
	static const named_traits *traits = 0;
	if (traits || (traits = client::pointer_traits())) {
		return &traits->traits;
	}
	return 0;
}

/*!
 * \ingroup mptClient
 * \brief get client pointer traits
 * 
 * Get name traits for client pointer
 * 
 * \return named traits for client pointer
 */
const struct named_traits *client::pointer_traits()
{
	return mpt_client_type_traits();
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
	return *cfg;
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
int client::convert(type_t type, void *ptr)
{
	int me = type_properties<client *>::id(true);
	if (me < 0) {
		me = TypeMetaPtr;
	}
	else if (assign(this, type, ptr)) {
		return TypeConfigPtr;
	}
	if (!type) {
		static const uint8_t fmt[] = { TypeConfigPtr, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	int cfg = type_properties<class config *>::id(true);
	if ((cfg > 0) && (type == static_cast<type_t>(cfg))) {
		if (ptr) *static_cast<class config **>(ptr) = clientConfig();
		return me;
	}
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		return TypeConfigPtr;
	}
	return BadType;
}

/*!
 * \ingroup mptClient
 * \brief dispatch event to client
 * 
 * Process command type messages.
 * 
 * \param ev  event to process
 * 
 * \return dispatch opertation result
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
