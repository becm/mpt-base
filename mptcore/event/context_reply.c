/*!
 * send event to return/error channel.
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

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
extern int mpt_context_reply(MPT_INTERFACE(reply_context) *rc, int code, const char *fmt, ...)
{
	va_list va;
	
	if (code < CHAR_MIN || code > CHAR_MAX) {
		return MPT_ERROR(BadArgument);
	}
	if (!rc) {
		const char *ansi = 0, *desc;
		
		if (!fmt) {
			return 0;
		}
		if (!code) {
			code = MPT_LOG(Debug);
		}
		else if (code > 0) {
			code = MPT_LOG(Info);
		}
		else {
			code = MPT_LOG(Error);
		}
		if ((isatty(fileno(stderr)) > 0) && (ansi = mpt_ansi_code(code))) {
			fputs(ansi, stderr);
		}
		fputc('[', stderr);
		if (!ansi && (desc = mpt_log_identifier(code))) {
			fputs(desc, stderr);
		}
		fputc('>', stderr);
		fputc(']', stderr);
		fputc(' ', stderr);
		
		if (ansi) {
			fputs(mpt_ansi_reset(), stderr);
		}
		va_start(va, fmt);
		vfprintf(stderr, fmt, va);
		va_end(va);
		fputc('\n', stderr);
		
		return 1;
	}
	else {
		char buf[256];
		MPT_STRUCT(message) msg;
		struct iovec cont;
		MPT_STRUCT(msgtype) mt;
		int ret;
		
		mt.cmd = MPT_ENUM(MessageAnswer);
		mt.arg = code;
		
		msg.base = &mt;
		msg.used = sizeof(mt);
		msg.clen = 0;
		
		if (fmt) {
			int len;
			va_start(va, fmt);
			len = vsnprintf(buf, sizeof(buf), fmt, va);
			va_end(va);
			if (len > 0) {
				if (len > (int) sizeof(buf)) {
					len = sizeof(buf);
					buf[len-1] = 0;
				}
				cont.iov_base = buf;
				cont.iov_len  = len;
				msg.cont = &cont;
				msg.clen = 1;
			}
		}
		if ((ret = rc->_vptr->reply(rc, &msg)) < 0) {
			return ret;
		}
		return 0;
	}
}
