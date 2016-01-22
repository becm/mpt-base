/*!
 * print arguments to output/error descriptors.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief send extended message
 * 
 * Use output log operation for extended message.
 * 
 * \param out  output descriptor
 * \param from message source information
 * \param type text message type
 * \param fmt  argument format
 * 
 * \return result of push operation
 */
extern int mpt_output_log(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, ... )
{
	MPT_INTERFACE(logger) *log;
	va_list ap;
	int err = 0;
	
	if (!out) {
		if (!(type & 0xff)) return 0;
		log = _mpt_log_default(0);
	} else {
		log = mpt_object_logger((MPT_INTERFACE(object) *) out);
	}
	if (log) {
		va_start(ap, fmt);
		err = log->_vptr->log(log, from, type, fmt, ap);
		va_end(ap);
		return err;
	}
	if (!out) {
		return 0;
	}
	if (from || fmt) {
		uint8_t hdr[2];
		hdr[0] = MPT_ENUM(MessageOutput);
		hdr[1] = type;
		
		if (out->_vptr->push(out, sizeof(hdr), hdr) < 0) {
			return -1;
		}
		if (from && out->_vptr->push(out, strlen(from)+1, from) < 0) {
			out->_vptr->push(out, 1, 0);
			return -1;
		}
		if (fmt) {
			char buf[1024];
			
			va_start(ap, fmt);
			err = vsnprintf(buf, sizeof(buf)-1, fmt, ap);
			va_end(ap);
			
			if (err < 0) {
				return err;
			}
			if (err > (int) (sizeof(buf) - 3)) {
				buf[sizeof(buf)-3] = buf[sizeof(buf)-2] = buf[sizeof(buf)-1] = '.';
				err = sizeof(buf);
			}
			if (out->_vptr->push(out, err, buf) < 0) {
				out->_vptr->push(out, 1, 0);
			}
			return -1;
		}
		if (out->_vptr->push(out, 0, 0) < 0) {
			out->_vptr->push(out, 1, 0);
		}
	}
	return 0;
}

