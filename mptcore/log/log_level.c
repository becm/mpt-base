
#include <string.h>
#include <strings.h>

#include "core.h"

/*!
 * \ingroup mptLog
 * \brief get output type
 * 
 * Get output type code from description.
 * 
 * \param type message type description
 * \param fd   log level code
 * 
 * \return prefix text
 */
extern int mpt_log_level(const char *arg)
{
	if (!arg || !*arg) {
		return MPT_ENUM(LogLevelWarning);
	}
	if (!strcasecmp(arg, "none")) {
		return MPT_ENUM(LogLevelNone);
	}
	if (!strcasecmp(arg, "fatal") || !strcasecmp(arg, "critical")) {
		return MPT_ENUM(LogLevelCritical);
	}
	if (!strcasecmp(arg, "error")) {
		return MPT_ENUM(LogLevelError);
	}
	if (!strcasecmp(arg, "warning") || !strcasecmp(arg, "default")) {
		return MPT_ENUM(LogLevelWarning);
	}
	if (!strcasecmp(arg, "info")) {
		return MPT_ENUM(LogLevelInfo);
	}
	if (!strcasecmp(arg, "debug") || !strcasecmp(arg, "debug1")) {
		return MPT_ENUM(LogLevelDebug1);
	}
	if (!strcasecmp(arg, "debug2")) {
		return MPT_ENUM(LogLevelDebug2);
	}
	if (!strcasecmp(arg, "debug3")) {
		return MPT_ENUM(LogLevelDebug3);
	}
	return -2;
}
