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
		arg = mpt_message_iterator(ev->msg, 0);
	}
	else {
		MPT_STRUCT(array) a = MPT_ARRAY_INIT;
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(message) msg;
		size_t part;
		int len;
		
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
		if ((len = mpt_array_message(&a, &msg, mt.arg)) < 0) {
			return 0;
		}
		arg = mpt_meta_arguments(&a);
		mpt_array_clone(&a, 0);
	}
	if (!arg) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), MPT_tr("no command content"));
	}
	return arg;
}
