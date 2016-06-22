
#include <stdlib.h>

#include <unistd.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief clear outdata connection
 * 
 * Clear connection resources of outdata type
 * 
 * \param out  outdata pointer
 */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *out)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(stream) *srm;
	size_t len;
	
	/* clear remaining contexts */
	if ((buf = out->_ctx._buf)) {
		if ((len = buf->used)) {
			MPT_STRUCT(reply_context) **r = (void *) (buf+1);
			mpt_reply_clear(r, len / sizeof(*r));
		}
	}
	/* close connection */
	if (out->sock._id >= 0) {
		/* keep std{in,out,err} */
		if (out->sock._id > 2) {
			(void) close(out->sock._id);
		}
		mpt_array_clone((MPT_STRUCT(array) *) &out->_buf, 0);
		out->sock._id = -1;
		out->_socklen = 0;
	}
	else if ((srm = out->_buf)) {
		mpt_stream_close(srm);
		free(srm);
		out->_buf = 0;
	}
	out->state &= 0xf | MPT_ENUM(OutputPrintColor);
}
/*!
 * \ingroup mptOutput
 * \brief clear outdata resources
 * 
 * Clear all resources of outdata type
 * 
 * \param out  outdata pointer
 */
extern void mpt_outdata_fini(MPT_STRUCT(outdata) *out)
{
	mpt_outdata_close(out);
	mpt_array_clone(&out->_ctx, 0);
	
	out->state = 0;
	out->_coding = 0;
	out->_idlen = 0;
}
