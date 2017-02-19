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
 * Push plot header for double precission floating point data.
 * 
 * \param out  output descriptor
 * \param dst  plot target
 * \param cyc  target cycle
 * \param off  cycle offset
 */
extern int mpt_output_init_plot(MPT_INTERFACE(output) *out, MPT_STRUCT(msgdest) dest, uint8_t fmt, const MPT_STRUCT(msgworld) *pos)
{
	MPT_STRUCT(msgtype) *type;
	MPT_STRUCT(msgworld) *wld;
	uint8_t hdr[sizeof(*type) + sizeof(dest) + sizeof(*wld)];
	
	wld = (void *) (hdr + sizeof(*type) + sizeof(dest));
	type = (void *) (hdr);
	
	type->cmd = MPT_ENUM(MessageDest);
	type->arg = (int8_t) fmt;
	memcpy(hdr+sizeof(*type), &dest, sizeof(dest));
	
	if (pos) {
		memcpy(wld, pos, sizeof(*wld));
	} else {
		wld->cycle = 0;
		wld->offset = 0;
	}
	return out->_vptr->push(out, sizeof(hdr), &hdr);
}
