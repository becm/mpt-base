
#include <stdio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief get file type
 * 
 * Determine file descriptor for message of
 * specified severity and limit.
 * 
 * \param arg  type of message
 * \param min  output limit for messages
 * 
 * \return output file descriptor
 */
extern int mpt_output_file(uint8_t arg, int min)
{
	int skip;
	
	if (arg & MPT_ENUM(LogFile)) {
		if (arg & 0x7f) return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore);
		return MPT_ENUM(OutputPrintHistory);
	}
	if (!arg) return MPT_ENUM(OutputPrintNormal);
	
	switch (min & 0x7) {
	  case MPT_ENUM(OutputLevelNone):     skip = MPT_ENUM(LogCritical); break;
	  case MPT_ENUM(OutputLevelCritical): skip = MPT_ENUM(LogError);    break;
	  case MPT_ENUM(OutputLevelError):    skip = MPT_ENUM(LogWarning);  break;
	  case MPT_ENUM(OutputLevelWarning):  skip = MPT_ENUM(LogInfo);     break;
	  case MPT_ENUM(OutputLevelInfo):     skip = MPT_ENUM(LogDebug);    break;
	  case MPT_ENUM(OutputLevelDebug1):   skip = MPT_ENUM(LogDebug2);   break;
	  case MPT_ENUM(OutputLevelDebug2):   skip = MPT_ENUM(LogDebug3);   break;
	  default: skip = MPT_ENUM(LogFile); /* MPT_ENUM(OutputLevelDebug3) */
	}
	
	if (min & MPT_ENUM(OutputLevelLog)) {
		if ((arg & 0x7f) < skip) return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore) | MPT_ENUM(OutputPrintNormal);
		return 0;
	}
	return (arg & 0x7f) < skip ? MPT_ENUM(OutputPrintError) : 0;
}
