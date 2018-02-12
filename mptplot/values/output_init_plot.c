/*!
 * send send solution data to output
 */

#include <string.h>

#include "output.h"
#include "message.h"

#include "values.h"

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
extern int mpt_output_init_plot(MPT_INTERFACE(output) *out, MPT_STRUCT(laydest) dest, uint8_t fmt, const MPT_STRUCT(valdest) *pos)
{
	MPT_STRUCT(msgtype) *type;
	MPT_STRUCT(valdest) *vd;
	uint8_t hdr[sizeof(*type) + sizeof(dest) + sizeof(*vd)];
	ssize_t ret;
	
	vd = (void *) (hdr + sizeof(*type) + sizeof(dest));
	type = (void *) (hdr);
	
	type->cmd = MPT_MESGTYPE(Destination);
	type->arg = (int8_t) fmt;
	memcpy(hdr+sizeof(*type), &dest, sizeof(dest));
	
	if (pos) {
		memcpy(vd, pos, sizeof(*vd));
	} else {
		vd->cycle = -1;
		vd->offset = 0;
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
