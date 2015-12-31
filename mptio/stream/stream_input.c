/*!
 * stream controller and creator.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <poll.h>

#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>

#include "convert.h"
#include "event.h"
#include "queue.h"

#include "notify.h"
#include "stream.h"

struct streamInput {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(stream)    data;
	size_t                idlen;
};

static void streamUnref(MPT_INTERFACE(input) *in)
{
	struct streamInput *srm = (void *) in;
	(void) mpt_stream_close(&srm->data);
	free(srm);
}
static int streamNext(MPT_INTERFACE(input) *in, int what)
{
	struct streamInput *srm = (void *) in;
	return mpt_stream_poll(&srm->data, what, -1);
}
static int streamDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct streamInput *srm = (void *) in;
	
	if (!cmd) {
		ssize_t avail = mpt_queue_peek(&srm->data._rd, &srm->data._dec.info, srm->data._dec.fcn,  0);
		if (avail >= 0) {
			return POLLIN;
		}
		if (_mpt_stream_fread(&srm->data._info) < 0) {
			return -2;
		}
		return 0;
	}
	return mpt_stream_dispatch(&srm->data, srm->idlen, cmd, arg);
}
static int streamFile(MPT_INTERFACE(input) *in)
{
	struct streamInput *srm = (void *) in;
	return _mpt_stream_fread(&srm->data._info);
}

static const MPT_INTERFACE_VPTR(input) streamCtl = {
	streamUnref,
	
	streamNext,
	streamDispatch,
	streamFile
};

/*!
 * \ingroup mptStream
 * \brief create stream input
 * 
 * Create input from stream information.
 * 
 * \param from  stream descriptor to convert
 * \param code  coding of input stream
 * 
 * \return created input
 */
extern MPT_INTERFACE(input) *mpt_stream_input(const MPT_STRUCT(socket) *from, int mode, int code, size_t idlen)
{
	MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
	struct streamInput *srm;
	
	/* input required */
	if ((mode & MPT_ENUM(StreamWrite))
	    && !(mode & MPT_ENUM(StreamRdWr))) {
		errno = EINVAL;
		return 0;
	}
	if (!code) {
		/* id requires message format on stream */
		if (idlen) {
			errno = EINVAL;
			return 0;
		}
	}
	else {
		/* input codec for message */
		if (!(tmp._dec.fcn = mpt_message_decoder(code))) {
			return 0;
		}
		/* bidirectional codec for reply */
		if ((mode & MPT_ENUM(StreamWrite))
		    && !(tmp._enc.fcn = mpt_message_encoder(code))) {
			return 0;
		}
		memset(&tmp._dec.info, 0, sizeof(tmp._dec.info));
		memset(&tmp._enc.info, 0, sizeof(tmp._enc.info));
	}
	/* set temporary data, no resource leakage on error */
	if ((mpt_stream_dopen(&tmp, from, mode) < 0)
	    || (!(srm = malloc(sizeof(*srm))))) {
		return 0;
	}
	srm->_in._vptr = &streamCtl;
	srm->data = tmp;
	srm->idlen = idlen;
	
	return &srm->_in;
}
