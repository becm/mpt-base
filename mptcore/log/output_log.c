/*!
 * print arguments to output/error descriptors.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/uio.h>

#include "output.h"

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
	MPT_INTERFACE(logger) *log;
	va_list ap;
	int err = 0;
	
	if (!out) {
		if (!(type & 0xff)) return 0;
		log = mpt_log_default();
	}
	if (fmt) {
		va_start(ap, fmt);
	}
	if (type & 0x7f && !(type & MPT_LOG(File))) {
		type |= MPT_ENUM(LogPretty) | MPT_ENUM(LogFunction);
	}
	if (out) {
		MPT_STRUCT(value) val;
		char buf[MPT_OUTPUT_LOGMSG_MAX];
		int len;
		len = vsnprintf(buf, sizeof(buf), fmt, ap);
		if (len > (int) sizeof(buf)) {
			buf[sizeof(buf) - 1] = 0; /* indicate truncation */
		}
		val.fmt = 0;
		val.ptr = buf;
		err = out->_vptr->log(out, from, type, &val);
	} else if (log) {
		err = log->_vptr->log(log, from, type, fmt, ap);
	}
	if (fmt) {
		va_end(ap);
	}
	return err;
}

