/*!
 * send event to return/error channel.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief event reply
 * 
 * Send text message as reply for event.
 * 
 * \param ev   event data
 * \param code message type
 * \param data message text
 * 
 * \return result of send operation
 */
extern int mpt_event_reply(const MPT_STRUCT(event) *ev, int code, const char *data)
{
	if (code < CHAR_MIN || code > CHAR_MAX) {
		errno = ERANGE;
		return -2;
	}
	
	if (!ev || !ev->reply.set) {
		FILE *fd = stderr;
		const char *ansi = 0;
		
		if (!code) {
			code = MPT_ENUM(LogDebug);
		}
		else if (code > 0) {
			code = MPT_ENUM(LogInfo);
		}
		else {
			code = MPT_ENUM(LogError);
		}
		
		if (!data) {
			return 0;
		}
		if ((isatty(fileno(fd)) > 0) && (ansi = mpt_ansi_code(code))) {
			fputs(ansi, fd);
		}
		fputc('[', fd);
		fputc('>', fd);
		fputs(mpt_message_identifier(code), fd);
		fputc(']', fd);
		fputc(' ', fd);
		
		if (ansi) {
			fputs(mpt_ansi_restore(), stderr);
		}
		fputs(data, fd);
		fputc('\n', stderr);
		
		return 1;
	}
	else {
		MPT_STRUCT(message) msg;
		struct iovec cont;
		MPT_STRUCT(msgtype) mt;
		int ret;
		
		mt.cmd = MPT_ENUM(MessageAnswer);
		mt.arg = code;
		
		msg.base = &mt;
		msg.used = sizeof(mt);
		
		
		if (data && (cont.iov_len = strlen(data))) {
			cont.iov_base = (void *) data;
			msg.cont = &cont;
			msg.clen = 1;
		} else {
			msg.clen = 0;
		}
		
		if ((ret = ev->reply.set(ev->reply.context, &msg)) < 0) {
			return ret;
		}
		return 0;
	}
}
