/*!
 * configure and prepare bound solver.
 */

#include <string.h>
#include <stdlib.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "client.h"

struct messageMetatype
{
	MPT_INTERFACE(metatype) _base;
	char sep;
	unsigned int len : 24;
	unsigned int pos : 32;
};

static void messageUnref(MPT_INTERFACE(metatype) *src)
{
	free(src);
}
static int messageAssign(MPT_INTERFACE(metatype) *src, const MPT_STRUCT(value) *val)
{
	(void) src;
	(void) val;
	return MPT_ERROR(BadOperation);
}
static int messageConv(MPT_INTERFACE(metatype) *src, int type, void *data)
{
	struct messageMetatype *s = (void *) src;
	MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
	ssize_t len;
	
	if (!(msg.used = s->len - s->pos)) {
		return MPT_ERROR(MissingData);
	}
	msg.base = ((char *) (src+1)) + s->pos;
	
	if ((len = mpt_message_argv(&msg, s->sep)) < 0) {
		return len;
	}
	
	if ((type & 0xff) == MPT_ENUM(TypeProperty)) {
		char *sep;
		
		if (!(type & MPT_ENUM(ValueConsume))) {
			return MPT_ERROR(BadArgument);
		}
		if (!(sep = memchr(msg.base, '=', len))) {
			return MPT_ERROR(BadValue);
		}
		*sep = 0;
		if (data) {
			MPT_STRUCT(property) *pr = data;
			
			pr->name = msg.base;
			pr->desc = 0;
			pr->val.fmt = 0;
			pr->val.ptr = sep + 1;
		}
	}
	else if (type != 's') {
		return MPT_ERROR(BadType);
	}
	else if (!(type & MPT_ENUM(ValueConsume))) {
		return MPT_ERROR(BadArgument);
	}
	else if (data) {
		*((const char **) data) = msg.base;
	}
	if (s->sep) {
		((char *) msg.base)[len] = 0;
	}
	s->pos = ((char *) msg.base - ((char *) (s+1))) + len;
	
	return type & (MPT_ENUM(ValueConsume) | 0xff);
}
static MPT_INTERFACE(metatype) *messageClone(MPT_INTERFACE(metatype) *src)
{
	struct messageMetatype *c, *msg = (void *) src;
	size_t len = sizeof(*msg) + msg->len;
	
	if (!(c = malloc(len))) {
		return 0;
	}
	return memcpy(c, msg, len);
}
static const MPT_INTERFACE_VPTR(metatype) messageCtl = {
	messageUnref,
	messageAssign,
	messageConv,
	messageClone
};

/*!
 * \ingroup mptClient
 * \brief create message metatype
 * 
 * Create metatype from message data to access arguments from
 * property assign interaction.
 * 
 * \param sep  argument separator
 * \param msg  message data
 * 
 * \return hint to event controller (int)
 */
extern MPT_INTERFACE(metatype) *mpt_message_metatype(int sep, const MPT_STRUCT(message) *ptr)
{
	struct messageMetatype *m;
	MPT_STRUCT(message) msg;
	size_t len;
	
	if (!ptr) {
		return 0;
	}
	len = mpt_message_length(&msg);
	if (!(m = malloc(sizeof(*m) + len + 1))) {
		return 0;
	}
	m->sep = sep;
	m->len = len;
	m->pos = 0;
	
	msg = *ptr;
	mpt_message_read(&msg, len, m+1);
	
	return &m->_base;
}
