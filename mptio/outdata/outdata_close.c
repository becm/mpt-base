
#include <stdlib.h>

#include <unistd.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief close outdata connection
 * 
 * Clear resources of outdata socket and
 * reset size information and data state.
 * 
 * \param out  outdata pointer
 */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *out)
{
	/* close connection */
	if (out->sock._id >= 0) {
		/* keep std{in,out,err} */
		if (out->sock._id > 2) {
			(void) close(out->sock._id);
		}
	}
	mpt_array_clone(&out->buf, 0);
	out->sock._id = -1;
	out->state &= 0xf;
	out->_smax = 0;
	out->_scurr = 0;
}
