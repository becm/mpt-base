/*!
 * set output bindings from description string.
 */

#include <string.h>
#include <limits.h>

#include <sys/uio.h>

#include "message.h"
#include "meta.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief assign data bindings
 * 
 * Send data bindings in text data to output.
 * 
 * \param out   output descriptor
 * \param descr text containing data bindings
 * 
 * \return number of bindings
 */
extern int mpt_outbind_string(MPT_INTERFACE(output) *out, const char *descr)
{
	MPT_STRUCT(strdest) str;
	MPT_STRUCT(msgtype) mt;
	struct {
		MPT_STRUCT(msgbind) src;
		MPT_STRUCT(msgdest) dst;
	} bnd;
	int len, dim = 1;
	
	if (!out) {
		return MPT_ERROR(BadArgument);
	}
	
	bnd.src.type = MPT_ENUM(DataStateAll);
	bnd.src.dim  = 0;
	
	bnd.dst.lay = bnd.dst.grf = bnd.dst.wld = 1;
	bnd.dst.dim = 0;
	
	str.change = 3;
	
	mt.cmd = MPT_ENUM(MessageGraphic);
	mt.arg = MPT_ENUM(BindingAdd);
	
	if (!descr) {
		mt.arg = MPT_ENUM(BindingClear);
		if (out->_vptr->await(out, 0, 0) < 0) {
			return -2;
		}
		return out->_vptr->push(out, sizeof(mt), &mt);
	}
	/* read binding description string */
	while ((len = mpt_string_dest(&str, ':', descr))) {
		if (len < 0) {
			(void) mpt_output_log(out, __func__, MPT_LOG(Error), "%s: %d: %s",
			                      MPT_tr("bad data destination"), bnd.src.dim+1, descr);
			return dim - 1;
		}
		descr += len;
		if (!str.change) {
			(void) mpt_output_log(out, __func__, MPT_LOG(Error), "%s: %d: %s",
			                       MPT_tr("identical data destination"), bnd.src.dim+1, descr-len);
			continue;
		}
		if (str.change & 1) bnd.dst.lay = str.val[0];
		if (str.change & 2) bnd.dst.grf = str.val[1];
		if (str.change & 4) bnd.dst.wld = str.val[2];
		
		if (!bnd.dst.lay || !bnd.dst.grf || !bnd.dst.wld) {
			(void) mpt_output_log(out, __func__, MPT_LOG(Error), "%s: %d: %s",
			                       MPT_tr("illegal data destination"), bnd.src.dim+1, descr - len);
			continue;
		}
		/* register/write header on first occasion */
		if (mt.cmd) {
			if (out->_vptr->await(out, 0, 0) < 0) {
				return -2;
			}
			if ((len = out->_vptr->push(out, sizeof(mt), &mt)) < 0) {
				return len;
			}
			mt.cmd = 0;
		}
		/* append x/y data bindings */
		bnd.src.dim = 0;
		bnd.dst.dim = 0;
		if ((len = out->_vptr->push(out, sizeof(bnd), &bnd)) < 0) break;
		bnd.src.dim = dim;
		bnd.dst.dim = 1;
		if ((len = out->_vptr->push(out, sizeof(bnd), &bnd)) < 0) break;
		
		str.change = 3;
		
		if ((len = dim++) == UINT8_MAX) break;
	}
	/* clear message data and registration */
	if (len < 0 || out->_vptr->push(out, 0, 0) < 0) {
		out->_vptr->push(out, 1, 0);
		return -1;
	}
	return len;
}

