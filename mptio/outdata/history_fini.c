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
	
	if (hist->pass) {
		/* restore downstream output */
		if (hist->state & MPT_ENUM(OutputRemote)) {
			hist->pass->_vptr->push(hist->pass, 1, 0);
		}
		hist->pass->_vptr->obj.ref.unref((void *) hist->pass);
		hist->pass = 0;
	}
}
