/*!
 * MPT I/O library
 *   open connection to target
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "message.h"
#include "convert.h"
#include "output.h"

#include "stream.h"

#include "connection.h"

/*!
 * \ingroup mptConnection
 * \brief set connection target
 * 
 * Open target stream/socket and assign to connection.
 * 
 * \param out  connection descriptor
 * \param to   stream/socket target
 * \param mp   connection type description
 */
extern int mpt_connection_open(MPT_STRUCT(connection) *con, const char *to, const MPT_STRUCT(fdmode) *mp)
{
	MPT_STRUCT(fdmode) mode;
	MPT_STRUCT(socket) tmp = MPT_SOCKET_INIT;
	MPT_STRUCT(stream) *srm = 0;
	int flg = 0, ret;
	
	if (con->out.state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(BadOperation);
	}
	if (!to) {
		return MPT_ERROR(BadArgument);
	}
	/* get mode from target string */
	if (!mp) {
		if ((flg = mpt_mode_parse(&mode, to)) < 0) {
			return flg;
		}
		mode.stream |= MPT_STREAMFLAG(Buffer);
		mp = &mode;
	}
	/* create new connection */
	if ((ret = mpt_connect(&tmp, to+flg, mp)) < 0) {
		return MPT_ERROR(BadValue);
	}
	if ((flg = mpt_stream_sockflags(ret)) >= 0) {
		static const char encoding = MPT_ENUM(EncodingCobs);
		MPT_STRUCT(stream) s = MPT_STREAM_INIT;
		MPT_TYPE(data_encoder) enc;
		MPT_TYPE(data_decoder) dec;
		
		if (!(enc = mpt_message_encoder(encoding))
		    || !(dec = mpt_message_decoder(encoding))) {
			return MPT_ERROR(BadEncoding);
		}
		if (mpt_stream_dopen(&s, &tmp, mp->stream) < 0) {
			(void) close(tmp._id);
			return MPT_ERROR(BadOperation);
		}
		s._wd._enc = enc;
		s._rd._dec = dec;
		
		if (!(srm = malloc(sizeof(*srm)))) {
			return MPT_ERROR(BadOperation);
		}
		*srm = s;
	}
	/* close old connection */
	mpt_connection_close(con);
	
	/* set new connection parameters */
	if (srm) {
		con->out.buf._buf = (void *) srm;
	} else {
		con->out.sock = tmp;
	}
	return ret;
}
