/*!
 * selective output for floating point data.
 */

#include <string.h>
#include <sys/uio.h>

#include "message.h"

#include "output.h"

#define MPT_fmtHdrLen(m) (sizeof(m.type) + m.type.arg*sizeof(m.bnd[0]))

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
	struct {
		MPT_STRUCT(msgtype)   type;
		MPT_STRUCT(msgvalfmt) bnd[2];
	} hdr;
	int plen, msgs, i, ldr, ldv;
	
	if (len < 0) return -1;
	
	if ((ldr = rlen) < 0 || (ldv = vlen) < 0) {
		return -2;
	}
	if (vlen && !val) {
		ldr = ldv = rlen + vlen;
		val = ref + rlen;
	}
	plen = rlen + vlen;
	
	if (len && plen) {
		uint64_t dim;
		uint8_t fmt;
		/* send size info */
		hdr.type.cmd = MPT_ENUM(MessageValFmt);
		hdr.type.arg = 0;
		
		fmt = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesInteger) | sizeof(dim);
		
		out->_vptr->push(out, sizeof(hdr.type), &hdr.type);
		out->_vptr->push(out, sizeof(fmt), &fmt);
		out->_vptr->push(out, sizeof(fmt), &fmt);
		fmt = 0;
		out->_vptr->push(out, sizeof(fmt), &fmt);
		dim = len;  out->_vptr->push(out, sizeof(dim), &dim);
		dim = plen; out->_vptr->push(out, sizeof(dim), &dim);
		out->_vptr->push(out, 0, 0);
	}
	/* header setup for data output */
	hdr.type.cmd = MPT_ENUM(MessageValFmt);
	hdr.type.arg = 2;
	
	hdr.bnd[0].fmt = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(*ref);
	hdr.bnd[0].len = ref ? rlen : 0;
	hdr.bnd[1].fmt = hdr.bnd[0].fmt;
	hdr.bnd[1].len = val ? vlen : 0;
	
	out->_vptr->push(out, MPT_fmtHdrLen(hdr), &hdr);
	msgs = 1;
	
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
		if ((4*1024) < ((i+1) * (MPT_fmtHdrLen(hdr) + rlen * sizeof(*ref) + vlen * sizeof(*val)))) {
			/* update for next reference step output */
			len -= i+1;
			i = -1;
			if (len) {
				out->_vptr->push(out, 0, 0);
				out->_vptr->push(out, MPT_fmtHdrLen(hdr), &hdr);
				++msgs;
			}
		}
	}
	/* finalize previous */
	out->_vptr->push(out, 0, 0);
	
	/* terminate history block */
	hdr.type.cmd = MPT_ENUM(MessageValFmt);
	out->_vptr->push(out, sizeof(hdr.type.cmd), &hdr.type.cmd);
	out->_vptr->push(out, 0, 0);
	
	return msgs;
}
