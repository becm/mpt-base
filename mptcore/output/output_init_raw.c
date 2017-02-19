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
 * Use default values for state flags and dimension.
 * 
 * \param out   output descriptor
 * \param fmt   data type for content
 * 
 * \return size of pushed data
 */
extern int mpt_output_init_raw(MPT_INTERFACE(output) *out, uint8_t fmt)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	int ret;
	
	hdr.mt.cmd = MPT_ENUM(MessageValRaw);
	hdr.mt.arg = fmt;
	
	hdr.bnd.dim = 0;
	hdr.bnd.state = 0;
	
	if ((ret = out->_vptr->push(out, sizeof(hdr), &hdr)) < 0) {
		return ret;
	}
	if (ret < (int) sizeof(hdr)) {
		return MPT_ERROR(MissingBuffer);
	}
	return ret;
}

