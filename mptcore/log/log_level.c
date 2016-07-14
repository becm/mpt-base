
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
		return MPT_LOG(LevelWarning);
	}
	if (!strcasecmp(arg, "none")) {
		return MPT_LOG(LevelNone);
	}
	if (!strcasecmp(arg, "fatal") || !strcasecmp(arg, "critical")) {
		return MPT_LOG(LevelCritical);
	}
	if (!strcasecmp(arg, "error")) {
		return MPT_LOG(LevelError);
	}
	if (!strcasecmp(arg, "warning") || !strcasecmp(arg, "default")) {
		return MPT_LOG(LevelWarning);
	}
	if (!strcasecmp(arg, "info")) {
		return MPT_LOG(LevelInfo);
	}
	if (!strcasecmp(arg, "debug") || !strcasecmp(arg, "debug1")) {
		return MPT_LOG(LevelDebug1);
	}
	if (!strcasecmp(arg, "debug2")) {
		return MPT_LOG(LevelDebug2);
	}
	if (!strcasecmp(arg, "debug3")) {
		return MPT_LOG(LevelDebug3);
	}
	return -2;
}
