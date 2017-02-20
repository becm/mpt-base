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
	ssize_t ret;
	
	if (len < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* special advance condition */
	if (ld != 1) {
		uint8_t buf[256];
		ssize_t take;
		int curr, parts = 0;
		do {
			take = sizeof(buf);
			curr = take / sizeof(*val);
			if (curr > len) {
				curr = len;
				take = curr * sizeof(*val);
			}
			/* make (partial) aligned data */
			mpt_copy64(curr, val, ld, buf, 1);
			/* push all aligned data */
			while (take) {
				if ((ret = out->_vptr->push(out, take, buf)) < 0) {
					return ret;
				}
				if (ret > take) {
					return MPT_ERROR(BadValue);
				}
				take -= ret;
			}
			len -= curr;
			val += curr * ld;
			++parts;
		} while (len);
		return parts;
	} else {
		size_t total = len * sizeof(*val);
		
		while (total) {
			if ((ret = out->_vptr->push(out, total, val)) < 0) {
				return ret;
			}
			if ((size_t) ret > total) {
				return MPT_ERROR(BadValue);
			}
			total -= ret;
		}
		return 1;
	}
}

