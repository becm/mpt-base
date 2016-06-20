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
#include "queue.h"
#include "array.h"
#include "event.h"

#include "notify.h"
#include "stream.h"

struct streamInput {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(stream)    data;
	MPT_STRUCT(array)     ctx;
	size_t                idlen;
};

static void streamUnref(MPT_INTERFACE(input) *in)
{
	struct streamInput *srm = (void *) in;
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = srm->ctx._buf)) {
		MPT_STRUCT(reply_context) **ctx = (void *) (buf+1);
		mpt_reply_clear(ctx, buf->used / sizeof(*ctx));
		mpt_array_clone(&srm->ctx, 0);
	}
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
	MPT_STRUCT(reply_context) *ctx = 0;
	
	if (srm->data._mlen < 0
	    && (srm->data._mlen = mpt_queue_recv(&srm->data._rd)) < 0) {
		if (_mpt_stream_fread(&srm->data._info) < 0) {
			return -2;
		}
		return 0;
	}
	if (!cmd) {
		return 1;
	}
	if (srm->data._rd._dec) {
		if (!(ctx = mpt_reply_reserve(&srm->ctx, srm->idlen))) {
			return -3;
		}
		ctx->len = srm->idlen;
		ctx->used = 1;
	}
	return mpt_stream_dispatch(&srm->data, ctx, cmd, arg);
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
		if (!(tmp._rd._dec = mpt_message_decoder(code))) {
			return 0;
		}
		/* bidirectional codec for reply */
		if ((mode & MPT_ENUM(StreamWrite))
		    && !(tmp._wd._enc = mpt_message_encoder(code))) {
			return 0;
		}
		memset(&tmp._rd._state, 0, sizeof(tmp._rd._state));
		memset(&tmp._wd._state, 0, sizeof(tmp._wd._state));
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
