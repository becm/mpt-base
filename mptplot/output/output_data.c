/*!
 * send send solution data to output
 */

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief send raw data
 * 
 * Send single dimension data via output descriptor.
 * 
 * \param out  output descriptor
 * \param mask data targets
 * \param dim  source dimension
 * \param len  number of elements
 * \param val  data address
 * \param ld   element advance
 * 
 * \return number of sent elements
 */
extern int mpt_output_data(MPT_INTERFACE(output) *out, int mask, int dim, int len, const double *val, int ld)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	ssize_t ret;
	
	if (len < 0) return -1;
	
	hdr.mt.cmd = MPT_ENUM(MessageValRaw);
	hdr.mt.arg = mask & 0xff;
	
	hdr.bnd.dim  = dim;
	hdr.bnd.type = MPT_message_value(Float, *val);
	
	/* send header */
	if ((ret = out->_vptr->push(out, sizeof(hdr), &hdr)) < 0) return ret;
	
	/* copy data */
	if (ld != 1) {
		uint8_t buf[1024];
		do {
			int curr = sizeof(buf)/sizeof(*val);
			if (curr > len) curr = len;
			/* make (partial) aligned data */
			mpt_copy64(curr, val, ld, buf, 1);
			if ((ret = out->_vptr->push(out, curr*sizeof(*val), buf)) < 0) break;
			len -= curr;
			val += curr * ld;
		} while (len);
		len = 1;
	} else {
		ret = out->_vptr->push(out, len*sizeof(*val), val);
		len = 0;
	}
	if (ret < 0 || (ret = out->_vptr->push(out, 0, 0)) < 0) {
		out->_vptr->push(out, 1, 0);
		return ret;
	}
	
	return len;
}

