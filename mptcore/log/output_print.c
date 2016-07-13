/*!
 * init/fini MPT dispatch descriptor.
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/uio.h>

#include "output.h"
#include "message.h"

extern int mpt_output_print(MPT_INTERFACE(output) *out, const MPT_STRUCT(message) *mptr)
{
	MPT_STRUCT(message) msg;
	char buf[128];
	size_t len;
	
	if (!mptr) {
		mpt_output_log(out, __func__, MPT_FCNLOG(Debug2), "%s",
		               MPT_tr("no message"));
		return 0;
	}
	msg = *mptr;
	len = mpt_message_read(&msg, 2, buf);
	
	if (len < 1) {
		mpt_output_log(out, __func__, MPT_FCNLOG(Info), "%s",
		               MPT_tr("empty message"));
		return 0;
	}
	if (buf[0] != MPT_ENUM(MessageOutput)
	    && buf[0] != MPT_ENUM(MessageAnswer)) {
		const char *fmt;
		/* print data info */
		switch (len + mpt_message_read(&msg, 1, buf+2)) {
		  case 1:  fmt = "{ %02x }"; break;
		  case 2:  fmt = "{ %02x, %02x }"; break;
		  case 3:  fmt = "{ %02x, %02x, %02x }"; break;
		  default: fmt = "{ %02x, %02x, ... }"; break;
		}
		len = snprintf(buf+2, sizeof(buf)-2, fmt, buf[0], buf[1], buf[2]);
		buf[0] = MPT_ENUM(MessageAnswer);
		buf[1] = 0;
		out->_vptr->push(out, len + 2, buf);
		out->_vptr->push(out, 0, 0);
		return 0;
	}
	out->_vptr->push(out, len, buf);
	while (1) {
		if (msg.used) {
			out->_vptr->push(out, msg.used, msg.base);
		}
		if (!msg.clen--) {
			break;
		}
		msg.base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		
		++msg.cont;
	}
	out->_vptr->push(out, 0, 0);
	return 0;
}
