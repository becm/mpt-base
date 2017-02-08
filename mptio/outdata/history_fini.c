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
	/* clear file output resources */
	mpt_array_clone(&hist->info._fmt, 0);
	mpt_array_clone(&hist->info._dat, 0);
	
	if (hist->file && hist->file != stdout && hist->file != stderr) {
		fclose(hist->file);
		hist->file = 0;
	}
}
