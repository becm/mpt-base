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
extern MPT_INTERFACE(iterator) *mpt_event_command(const MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(iterator) *arg;
	
	if (!ev->msg) {
		arg = mpt_message_iterator(ev->msg, 0);
	}
	else {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(message) msg;
		size_t part;
		
		msg = *ev->msg;
		if (!(part = mpt_message_read(&msg, sizeof(mt), &mt))) {
			mpt_context_reply(ev->reply, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return 0;
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadType), MPT_tr("bad message type"));
			return 0;
		}
		if (part < sizeof(mt)) {
			mt.arg = 0;
		}
		arg = mpt_message_iterator(&msg, mt.arg);
	}
	if (!arg) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), MPT_tr("no command content"));
	}
	return arg;
}
