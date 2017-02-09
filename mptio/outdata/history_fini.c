/*!
 * finalize connection data
 */

#include <stdio.h>

#include "array.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief clear history
 * 
 * Free history descriptor resources.
 * 
 * \param hist  history descriptor
 */
extern void mpt_history_fini(MPT_STRUCT(history) *hist)
{
	FILE *fd;
	/* clear file output resources */
	mpt_array_clone(&hist->fmt._fmt, 0);
	mpt_array_clone(&hist->fmt._dat, 0);
	
	if ((fd = hist->info.file) && fd != stdout && fd != stderr) {
		fclose(fd);
		hist->info.file = 0;
	}
}