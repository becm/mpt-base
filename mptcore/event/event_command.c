/*
 * convert event message to command
 */

#include "message.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief event command
 * 
 * Get gommand and arguments from event message.
 * Event reply is set ON ERROR.
 * 
 * \param ev  event data
 * 
 * \return metatype with command and arguments
 */
extern MPT_INTERFACE(metatype) *mpt_event_command(const MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *arg;
	
	if (!ev->msg) {
		arg = mpt_meta_message(ev->msg, 0);
	}
	else {
		MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
		MPT_STRUCT(message) msg;
		ssize_t part;
		
		msg = *ev->msg;
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			mpt_context_reply(ev->reply, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return 0;
		}
		else if (mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadType), MPT_tr("bad message type"));
			return 0;
		}
		arg = mpt_meta_message(&msg, mt.arg);
	}
	if (!arg) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), MPT_tr("no command content"));
	}
	return arg;
}
