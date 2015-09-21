
#include <string.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief message prefix
 * 
 * Determine text prefix for message of
 * specified severity for file descriptor.
 * 
 * \param type type of message
 * \param fd   output file descriptor
 * 
 * \return prefix text
 */
extern int mpt_output_level(const char *arg)
{
	if (!arg || !*arg) {
		return MPT_ENUM(OutputLevelWarning);
	}
	if (!strcasecmp(arg, "none")) {
		return MPT_ENUM(OutputLevelNone);
	}
	if (!strcasecmp(arg, "fatal") || !strcasecmp(arg, "critical")) {
		return MPT_ENUM(OutputLevelCritical);
	}
	if (!strcasecmp(arg, "error")) {
		return MPT_ENUM(OutputLevelError);
	}
	if (!strcasecmp(arg, "warning") || !strcasecmp(arg, "default")) {
		return MPT_ENUM(OutputLevelWarning);
	}
	if (!strcasecmp(arg, "info")) {
		return MPT_ENUM(OutputLevelInfo);
	}
	if (!strcasecmp(arg, "debug") || !strcasecmp(arg, "debug1")) {
		return MPT_ENUM(OutputLevelDebug1);
	}
	if (!strcasecmp(arg, "debug2")) {
		return MPT_ENUM(OutputLevelDebug2);
	}
	if (!strcasecmp(arg, "debug3")) {
		return MPT_ENUM(OutputLevelDebug3);
	}
	return -2;
}
