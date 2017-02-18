/*!
 * send send solution data to output
 */

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptMessage
 * \brief push value header
 * 
 * Push data header for raw value format to output.
 * 
 * \param out   output descriptor
 * \param fmt   data type for content
 * \param state data state
 * \param dim   source dimension
 * 
 * \return number of sent elements
 */
extern int mpt_output_init_raw(MPT_INTERFACE(output) *out, char fmt, int state, int dim)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	
	hdr.mt.cmd = MPT_ENUM(MessageValRaw);
	hdr.mt.arg = fmt;
	
	hdr.bnd.dim  = dim;
	hdr.bnd.state = state & 0xff;
	
	/* send header */
	return out->_vptr->push(out, sizeof(hdr), &hdr);
}

