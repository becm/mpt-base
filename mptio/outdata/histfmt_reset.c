/*!
 * set history state
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "array.h"
#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief reset history state
 * 
 * Clear history current output state.
 * 
 * \param hist  history info
 * 
 * \return zero on success
 */
extern void mpt_histfmt_reset(MPT_STRUCT(histfmt) *fmt)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = fmt->_dat._buf)) {
		buf->used = 0;
	}
	fmt->pos = 0;
	fmt->all = 0;
	fmt->fmt = 0;
}
