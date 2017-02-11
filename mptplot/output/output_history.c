/*!
 * selective output for floating point data.
 */

#include <string.h>
#include <sys/uio.h>

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \~english
 * \brief send history data
 * 
 * Send binary data to output descriptor.
 * Pass zero pointer for <b>val</b> for contigous reference and value data
 * 
 * \param out  output descriptor
 * \param len  number of history blocks
 * \param ref  address of reference data
 * \param rlen number of reference elements per block
 * \param val  address of value data
 * \param vlen number of value elements per block
 * 
 * \return number of used messages for data
 */
extern int mpt_output_history(MPT_INTERFACE(output) *out, int len, const double *ref, int rlen, const double *val, int vlen)
{
	MPT_STRUCT(msgtype) hdr;
	int plen, msgs, i, ldr, ldv;
	uint8_t fmt;
	
	if (len < 0) return -1;
	
	if ((ldr = rlen) < 0 || (ldv = vlen) < 0) {
		return -2;
	}
	if (vlen && !val) {
		ldr = ldv = rlen + vlen;
		val = ref + rlen;
	}
	plen = rlen + vlen;
	msgs = 0;
	
	if (len && plen) {
		uint64_t dim;
		/* send size info */
		hdr.cmd = MPT_ENUM(MessageValFmt);
		hdr.arg = 0;
		
		fmt = MPT_message_value(Unsigned, dim);
		
		out->_vptr->push(out, sizeof(hdr), &hdr);
		
		dim = len;
		out->_vptr->push(out, sizeof(fmt), &fmt);
		out->_vptr->push(out, sizeof(dim), &dim);
		dim = plen;
		out->_vptr->push(out, sizeof(fmt), &fmt);
		out->_vptr->push(out, sizeof(dim), &dim);
		
		out->_vptr->push(out, 0, 0);
		++msgs;
	}
	/* header setup for data output */
	hdr.cmd = MPT_ENUM(MessageValFmt);
	hdr.arg = plen;
	fmt = MPT_message_value(Float, *ref);
	
	out->_vptr->push(out, sizeof(hdr), &hdr);
	for (i = 0; i < plen; i++) {
		out->_vptr->push(out, sizeof(fmt), &fmt);
	}
	for (i = 0; i < len; i++) {
		/* send reference data */
		if (ref && rlen) {
			out->_vptr->push(out, rlen * sizeof(*ref), ref);
			ref += ldr;
		}
		/* send value data */
		if (val && vlen) {
			out->_vptr->push(out, vlen * sizeof(*val), val);
			val += ldv;
		}
		/* try to limit message size to 4k */
		if ((4*1024) < (sizeof(hdr) + plen + (i+1) * (rlen * sizeof(*ref) + vlen * sizeof(*val)))) {
			/* update for next reference step output */
			len -= i+1;
			i = -1;
			if (len) {
				int j;
				out->_vptr->push(out, 0, 0);
				out->_vptr->push(out, sizeof(hdr), &hdr);
				for (j = 0; j < plen; j++) {
					out->_vptr->push(out, sizeof(fmt), &fmt);
				}
				++msgs;
			}
		}
	}
	/* finalize previous */
	out->_vptr->push(out, 0, 0);
	++msgs;
	
	/* terminate history block */
	fmt = MPT_ENUM(MessageValFmt);
	out->_vptr->push(out, sizeof(fmt), &fmt);
	out->_vptr->push(out, 0, 0);
	
	return msgs;
}
