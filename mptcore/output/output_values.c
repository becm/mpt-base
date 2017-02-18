/*!
 * send send solution data to output
 */

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptMessage
 * \brief send double data
 * 
 * Push double values to output.
 * 
 * \param out  output descriptor
 * \param len  number of elements
 * \param val  data address
 * \param ld   element advance
 * 
 * \return number of sent elements
 */
extern int mpt_output_values(MPT_INTERFACE(output) *out, int len, const double *val, int ld)
{
	int ret;
	
	if (len < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* special advance condition */
	if (ld != 1) {
		uint8_t buf[1024];
		do {
			int curr = sizeof(buf)/sizeof(*val);
			if (curr > len) curr = len;
			/* make (partial) aligned data */
			mpt_copy64(curr, val, ld, buf, 1);
			if ((ret = out->_vptr->push(out, curr*sizeof(*val), buf)) < 0) {
				break;
			}
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

