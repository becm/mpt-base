
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
	if (type & MPT_LOG(File)) {
		type &= 0x7f;
		if (type >= MPT_LOG(Debug))    return "DEBUG";
		if (type >= MPT_LOG(Info))     return "INFO";
		if (type >= MPT_LOG(Warning))  return "WARNING";
		if (type >= MPT_LOG(Error))    return "ERROR";
		if (type >= MPT_LOG(Critical)) return "CRITICAL";
		if (type >= MPT_LOG(Fatal))    return "FATAL";
	} else {
		type &= 0x7f;
		if (type >= MPT_LOG(Debug))    return "debug";
		if (type >= MPT_LOG(Info))     return "info";
		if (type >= MPT_LOG(Warning))  return "warning";
		if (type >= MPT_LOG(Error))    return "error";
		if (type >= MPT_LOG(Critical)) return "critical";
		if (type >= MPT_LOG(Fatal))    return "fatal";
	}
	return 0;
}
