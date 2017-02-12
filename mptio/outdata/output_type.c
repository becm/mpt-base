
#include <stdio.h>

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief get print state
 * 
 * Determine print flags for message of
 * specified severity and limit.
 * 
 * \param arg  type of message
 * \param max  output limit for messages
 * 
 * \return outdata print state
 */
extern int mpt_output_type(uint8_t arg, int max)
{
	if (!arg) {
		return MPT_OUTFLAG(PrintNormal);
	}
	if (arg & MPT_LOG(File)) {
		if (arg & ~MPT_LOG(File)) {
			return MPT_OUTFLAG(PrintHistory) | MPT_OUTFLAG(PrintRestore);
		}
		return MPT_OUTFLAG(PrintHistory);
	}
	if (arg >= max) {
		return 0;
	}
	return MPT_OUTFLAG(PrintError);
}
