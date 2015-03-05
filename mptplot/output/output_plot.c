/*!
 * send send solution data to output
 */

#include <string.h>

#include "array.h"
#include "plot.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief push plot message
 * 
 * Append plot target message to output.
 * 
 * \param out  output descriptor
 * \param dst  plot target
 * \param len  number of values
 * \param val  value start address
 * \param ld   distance to next element
 */
extern int mpt_output_plot(MPT_INTERFACE(output) *out, const MPT_STRUCT(laydest) *dest, int len, const double *val, int ld)
{
	MPT_STRUCT(msgtype)  *type;
	MPT_STRUCT(msgworld) *wld;
	uint8_t hdr[sizeof(*type) + sizeof(*dest) + sizeof(*wld)];
	MPT_STRUCT(msgval) v;
	int total = 0;
	
	if (dest) {
		memcpy(hdr+sizeof(*type), dest, sizeof(*dest));
	} else {
		memset(hdr+sizeof(*type), 0, sizeof(*dest));
	}
	wld  = (void *) (hdr + sizeof(*type) + sizeof(*dest));
	type = (void *) (hdr);
	
	type->cmd = MPT_ENUM(MessageDest);
	type->arg = (int8_t) (MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(*val));
	wld->cycle  = 0;
	wld->offset = 0;
	
	v.base = val;
	v.elem = len;
	v.copy = (void (*)()) mpt_copy64;
	v.ld   = ld;
	
	while (v.elem) {
		ssize_t curr;
		
		if ((curr = out->_vptr->push(out, sizeof(hdr), &hdr)) >= 0) {
			if ((curr = mpt_output_values(out, &v, sizeof(*val))) >= 0) {
				total += curr;
				v.elem -= curr;
				val += curr * ld;
				wld->offset += curr;
				curr = out->_vptr->push(out, 0, 0);
			} else {
				out->_vptr->push(out, 1, 0);
			}
		}
		/* remove last message on error */
		if (curr < 0) {
			return total ? total : -2;
		}
	}
	return total;
}

