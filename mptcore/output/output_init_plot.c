/*!
 * send send solution data to output
 */

#include <string.h>

#include "output.h"

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief push plot header
 * 
 * Push header for plot data with destination to output.
 * 
 * \param out   output descriptor
 * \param dest  plot target
 * \param fmt   message value format
 * \param pos   cycle index and offset
 */
extern int mpt_output_init_plot(MPT_INTERFACE(output) *out, MPT_STRUCT(msgdest) dest, uint8_t fmt, const MPT_STRUCT(msgworld) *pos)
{
	MPT_STRUCT(msgtype) *type;
	MPT_STRUCT(msgworld) *wld;
	uint8_t hdr[sizeof(*type) + sizeof(dest) + sizeof(*wld)];
	ssize_t ret;
	
	wld = (void *) (hdr + sizeof(*type) + sizeof(dest));
	type = (void *) (hdr);
	
	type->cmd = MPT_MESGTYPE(Destination);
	type->arg = (int8_t) fmt;
	memcpy(hdr+sizeof(*type), &dest, sizeof(dest));
	
	if (pos) {
		memcpy(wld, pos, sizeof(*wld));
	} else {
		wld->cycle = 0;
		wld->offset = 0;
	}
	ret = out->_vptr->push(out, sizeof(hdr), &hdr);
	
	if (ret < 0) {
		return ret;
	}
	/* reset message compose state */
	if (ret < (ssize_t) sizeof(hdr)) {
		out->_vptr->push(out, 1, 0);
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
