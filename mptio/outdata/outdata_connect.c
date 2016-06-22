
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "queue.h"
#include "stream.h"

#include "array.h"
#include "message.h"

#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief set outdata backend
 * 
 * Open target stream/socket and assign to output data.
 * 
 * \param out  output data descriptor
 * \param to   stream/socket target
 * \param mp   connection type description
 */
extern int mpt_outdata_connect(MPT_STRUCT(outdata) *out, const char *to, MPT_STRUCT(fdmode) *mp)
{
	MPT_STRUCT(fdmode) mode;
	MPT_STRUCT(socket) tmp = MPT_SOCKET_INIT;
	MPT_STRUCT(stream) *srm = 0;
	int flg = 0;
	
	if (out->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(BadOperation);
	}
	if (!to) {
		mpt_outdata_close(out);
		return 0;
	}
	/* get mode from target string */
	if (!mp && (flg = mpt_mode_parse(mp = &mode, to)) < 0) {
		flg = 0;
		mp = 0;
	}
	/* create new connection */
	if ((flg = mpt_connect(&tmp, to+flg, mp)) < 0) {
		return MPT_ERROR(BadValue);
	}
	if (flg & MPT_ENUM(SocketStream)) {
		MPT_STRUCT(stream) s = MPT_STREAM_INIT;
		MPT_TYPE(DataEncoder) enc = 0;
		MPT_TYPE(DataDecoder) dec = 0;
		
		if (out->_coding
		    && (!(enc = mpt_message_encoder(out->_coding))
		        || !(dec = mpt_message_decoder(out->_coding)))) {
			return MPT_ERROR(BadEncoding);
		}
		if (mpt_stream_dopen(&s, &tmp, mode.stream) < 0) {
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
	mpt_outdata_close(out);
	
	/* set new connection parameters */
	if (srm) {
		out->_buf = srm;
		out->sock._id = -1;
	} else {
		out->sock = tmp;
	}
	return flg;
}
