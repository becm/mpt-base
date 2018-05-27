/*!
 * set history state
 */

#include "history.h"

/*!
 * \ingroup mptHistory
 * \brief reset history format
 * 
 * Clear history format info.
 * 
 * \param fmt  history format
 */
extern void mpt_histfmt_reset(MPT_STRUCT(histfmt) *fmt)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = fmt->_dat._buf)) {
		buf->_used = 0;
	}
	fmt->pos = 0;
	fmt->fmt = 0;
}
