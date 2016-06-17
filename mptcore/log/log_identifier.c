
#include <unistd.h>

#include "message.h"

/*!
 * \ingroup mptLog
 * \brief message type description
 * 
 * Determine text description for message type.
 * 
 * \param arg type of message
 * 
 * \return message type description
 */
extern const char *mpt_log_identifier(int type)
{
	if (type <= 0) {
		return 0;
	}
	if (type & MPT_ENUM(LogFile)) {
		type &= 0x7f;
		if (type >= MPT_ENUM(LogDebug))    return "DEBUG";
		if (type >= MPT_ENUM(LogInfo))     return "INFO";
		if (type >= MPT_ENUM(LogWarning))  return "WARNING";
		if (type >= MPT_ENUM(LogError))    return "ERROR";
		if (type >= MPT_ENUM(LogCritical)) return "CRITICAL";
		if (type >= MPT_ENUM(LogFatal))    return "FATAL";
	} else {
		type &= 0x7f;
		if (type >= MPT_ENUM(LogDebug))    return "debug";
		if (type >= MPT_ENUM(LogInfo))     return "info";
		if (type >= MPT_ENUM(LogWarning))  return "warning";
		if (type >= MPT_ENUM(LogError))    return "error";
		if (type >= MPT_ENUM(LogCritical)) return "critical";
		if (type >= MPT_ENUM(LogFatal))    return "fatal";
	}
	return 0;
}
