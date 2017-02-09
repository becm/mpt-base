/*!
 * print arguments to output/error descriptors.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/uio.h>

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptMessage
 * \brief send log message
 * 
 * Use output for log message creation.
 * 
 * \param out  output descriptor
 * \param from message source information
 * \param type text message type
 * \param fmt  argument format
 * \param args argument data
 * 
 * \return result of log operation
 */
extern int mpt_output_vlog(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, va_list ap)
{
	int8_t hdr[4];
	int len, flen;
	ssize_t ret;
	
	hdr[0] = MPT_ENUM(MessageOutput);
	hdr[1] = type & 0x7f;
	
	len = 2;
	flen = 0;
	
	if (from) {
		flen = strlen(from);
		if (type & MPT_ENUM(LogFunction)) {
			hdr[len++] = 0x1; /* SOH */
		}
	}
	ret = MPT_OUTPUT_LOGMSG_MAX;
	if (fmt) {
		ret -= 2;
	}
	if (flen >= (ret - len)) {
		return MPT_ERROR(MissingBuffer);
	}
	if ((ret = out->_vptr->push(out, len, hdr)) < 0) {
		return ret;
	}
	if (from) {
		out->_vptr->push(out, flen, from);
		len += flen;
	}
	if (!fmt) {
		hdr[0] = 0x4; /* EOT */
		out->_vptr->push(out, 1, hdr);
	}
	else {
		char buf[MPT_OUTPUT_LOGMSG_MAX];
		int rem;
		buf[0] = 0x2; /* STX */
		++len;
		rem = sizeof(buf) - len;
		len = vsnprintf(buf+1, rem, fmt, ap);
		
		if (len < 0) {
			return MPT_ERROR(BadOperation);
		}
		if (len < rem) {
			buf[++len] = 0x3; /* ETX */
		} else {
			len = rem;
		}
		out->_vptr->push(out, len+1, buf);
	}
	out->_vptr->push(out, 0, 0);
	
	return 1;
}

/*!
 * \ingroup mptMessage
 * \brief send log message
 * 
 * Use output log operation for extended message.
 * Fallback to default log descriptor for output.
 * 
 * \param out  output descriptor
 * \param from message source information
 * \param type text message type
 * \param fmt  argument format
 * 
 * \return result of log operation
 */
extern int mpt_output_log(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, ... )
{
	MPT_INTERFACE(logger) *log = 0;
	va_list ap;
	int ret;
	
	if (!out) {
		if (!(type & 0xff)) {
			return 0;
		}
		if (!(log = mpt_log_default())) {
			return MPT_ERROR(BadOperation);
		}
	}
	if (fmt) va_start(ap, fmt);
	if (out) {
		ret = mpt_output_vlog(out, from, type, fmt, ap);
	} else {
		log->_vptr->log(log, from, type, fmt, ap);
	}
	if (fmt) va_end(ap);
	
	return ret;
}
