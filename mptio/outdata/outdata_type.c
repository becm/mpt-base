
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
	int skip;
	
	if (arg & MPT_LOG(File)) {
		if (arg & 0x7f) return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore);
		return MPT_ENUM(OutputPrintHistory);
	}
	if (!arg) return MPT_ENUM(OutputPrintNormal);
	
	switch (min & 0x7) {
	  case MPT_LOG(LevelNone):     skip = MPT_LOG(Critical); break;
	  case MPT_LOG(LevelCritical): skip = MPT_LOG(Error);    break;
	  case MPT_LOG(LevelError):    skip = MPT_LOG(Warning);  break;
	  case MPT_LOG(LevelWarning):  skip = MPT_LOG(Info);     break;
	  case MPT_LOG(LevelInfo):     skip = MPT_LOG(Debug);    break;
	  case MPT_LOG(LevelDebug1):   skip = MPT_LOG(Debug2);   break;
	  case MPT_LOG(LevelDebug2):   skip = MPT_LOG(Debug3);   break;
	  default: skip = MPT_LOG(File);
	}
	
	if (min & MPT_LOG(LevelFile)) {
		if ((arg & 0x7f) < skip) return MPT_ENUM(OutputPrintHistory) | MPT_ENUM(OutputPrintRestore) | MPT_ENUM(OutputPrintNormal);
		return 0;
	}
	return (arg & 0x7f) < skip ? MPT_ENUM(OutputPrintError) : 0;
}
