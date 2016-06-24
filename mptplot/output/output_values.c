/*!
 * plot double values to output
 */

#include "message.h"

#include "output.h"

#define MPT_PART_SIZE  0x8000

/*!
 * \ingroup mptOutput
 * \brief push values to output
 * 
 * Append (transformed) values to currently constructed message.
 * 
 * \param out   output descriptor
 * \param val   value information
 * \param esize size of single value element
 * 
 * \return number of consumed elements
 */
extern int mpt_output_values(MPT_INTERFACE(output) *out, const MPT_STRUCT(output_values) *val, size_t esize)
{
	ssize_t wr;
	size_t len;
	
	if (!val) return 0;
	if (!esize) return -3;
	if (!(len = MPT_PART_SIZE / esize)) return -2;
	
	if (len > val->elem) {
		len = val->elem;
	}
	
	if (val->copy) {
		uint8_t buf[MPT_PART_SIZE];
		
		val->copy(len, val->base, val->ld, buf, 1);
		
		wr = out->_vptr->push(out, len * esize, buf);
	} else {
		wr = out->_vptr->push(out, len * esize, val->base);
	}
	return (wr < 0) ? wr : (int) len;
	
}
