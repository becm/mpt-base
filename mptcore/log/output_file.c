
#include <stdio.h>

#include "message.h"

/*!
 * \ingroup mptLog
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
	  case MPT_ENUM(LogLevelNone):     skip = MPT_ENUM(LogCritical); break;
	  case MPT_ENUM(LogLevelCritical): skip = MPT_ENUM(LogError);    break;
	  case MPT_ENUM(LogLevelError):    skip = MPT_ENUM(LogWarning);  break;
	  case MPT_ENUM(LogLevelWarning):  skip = MPT_ENUM(LogInfo);     break;
	  case MPT_ENUM(LogLevelInfo):     skip = MPT_ENUM(LogDebug);    break;
	  case MPT_ENUM(LogLevelDebug1):   skip = MPT_ENUM(LogDebug2);   break;
	  case MPT_ENUM(LogLevelDebug2):   skip = MPT_ENUM(LogDebug3);   break;
	  default: skip = MPT_ENUM(LogFile);
	}
	
	if (min & MPT_ENUM(LogLevelFile)) {
		if ((arg & 0x7f) < skip) return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore) | MPT_ENUM(OutputPrintNormal);
		return 0;
	}
	return (arg & 0x7f) < skip ? MPT_ENUM(OutputPrintError) : 0;
}
