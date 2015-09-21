/*!
 * print message data.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include <unistd.h>

#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief print message
 * 
 * Print message data or metadata to file.
 * 
 * \param out	file descriptor
 * \param omsg	message data
 * 
 * \return written length
 */
extern ssize_t mpt_message_print(FILE *out, const MPT_STRUCT(message) *omsg)
{
	MPT_STRUCT(msgtype) mt;
	MPT_STRUCT(message) msg;
	ssize_t len;
	int arg;
	const char *ansi = 0;
	
	if (!omsg) return 0;
	msg = *omsg;
	
	if ((len = mpt_message_read(&msg, sizeof(mt), &mt)) < 2) {
		if (!out) out = stderr;
		fprintf(out, "%s(): %s\n", __func__, MPT_tr("invalid message"));
		return -1;
	}
	arg = mt.arg;
	
	if (mt.cmd == MPT_ENUM(MessageAnswer)) {
		if (arg < 0) {
			arg = MPT_ENUM(LogError);
			if (!mpt_message_length(&msg)) {
				msg.base = "Error processing message";
				msg.used = strlen(msg.base);
			}
		}
		else if (arg) {
			arg = MPT_ENUM(LogDebug);
		}
	}
	else if (mt.cmd != MPT_ENUM(MessageOutput)) {
		if (!out) return 0;
		return fprintf(out, "%s {\"cmd\":%u, \"arg\":%i}\n", MPT_tr("message"), mt.cmd, mt.arg);
	}
	if (!mpt_message_length(&msg)) {
		return 0;
	}
	if (!out) {
		if (!mt.arg) return 0;
		out = stderr;
	}
	len = 0;
	
	if ((isatty(fileno(out))) && (ansi = mpt_ansi_code(arg))) {
		fputs(ansi, out);
	}
	if (mt.cmd == MPT_ENUM(MessageAnswer)) {
		fputc('@', out);
	}
	while (1) {
		if (msg.used) {
			const uint8_t *sep;
			size_t plen;
			if ((sep = memchr(msg.base, 0, msg.used)) && (plen = sep - ((uint8_t *) msg.base))) {
				fwrite(msg.base, plen, 1, out);
				msg.used -= ++plen;
				msg.base = ((uint8_t *) msg.base) + plen;
				fwrite(": ", 2, 1, out);
				len += ++plen;
				continue;
			}
			fwrite(msg.base, msg.used, 1, out);
			len += msg.used;
		}
		if (!msg.clen--) break;
		msg.base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		++msg.cont;
	}
	if (ansi) {
		fputs(mpt_ansi_restore(), out);
	}
	fputc('\n', out);
	
	return len;
}
