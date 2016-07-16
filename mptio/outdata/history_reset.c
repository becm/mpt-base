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
extern void mpt_history_reset(MPT_STRUCT(histinfo) *hist)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = hist->_dat._buf)) {
		buf->used = 0;
	}
	hist->pos.fmt = 0;
	hist->pos.elem = 0;
	hist->fpos = 0;
	hist->lfmt = 0;
}
