
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
 * \param min  output limit for messages
 * 
 * \return outdata print state
 */
extern int mpt_outdata_type(uint8_t arg, int min)
{
	if (!arg) {
		return MPT_ENUM(OutputPrintNormal);
	}
	if (arg & MPT_LOG(File)) {
		arg &= ~MPT_LOG(File);
		if (arg < MPT_LOG(File) || arg > MPT_LOG(Debug)) {
			return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore);
		}
		return MPT_ENUM(OutputPrintHistory);
	}
	if (arg < min) {
		return 0;
	}
	return MPT_ENUM(OutputPrintError);
}
