
#include <stdio.h>

#include <unistd.h>

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief clear outdata resources
 * 
 * Clear resources of outdata type
 * 
 * \param out  outdata pointer
 */
extern void mpt_outdata_fini(MPT_STRUCT(outdata) *out)
{
	if (out->sock._id >= 0) {
		(void) close(out->sock._id);
		out->sock._id = -1;
		out->_sflg = 0;
	}
	out->curr = 0;
	out->state &= 0xf | MPT_ENUM(OutputPrintColor);
	mpt_array_clone(&out->_buf, 0);
	
	if (out->hist
	    && (out->hist != stdin)
	    && (out->hist != stdout)
	    && (out->hist != stderr)) {
		fclose(out->hist);
		out->hist = 0;
	}
}
