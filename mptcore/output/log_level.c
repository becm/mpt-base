
#include <string.h>
#include <strings.h>

#include "output.h"

/*!
 * \ingroup mptLog
 * \brief get output type
 * 
 * Get output type code from description.
 * 
 * \param arg  log level code
 * 
 * \return prefix text
 */
extern int mpt_log_level(const char *arg)
{
	if (!arg || !*arg) {
		return MPT_LOG(Info) - 1;
	}
	if (!strcasecmp(arg, "none")) {
		return 0;
	}
	if (!strcasecmp(arg, "fatal")) {
		return MPT_LOG(Critical) - 1;
	}
	if (!strcasecmp(arg, "critical")) {
		return MPT_LOG(Error) - 1;
	}
	if (!strcasecmp(arg, "error")) {
		return MPT_LOG(Warning) - 1;
	}
	if (!strcasecmp(arg, "warning") || !strcasecmp(arg, "default")) {
		return MPT_LOG(Info) - 1;
	}
	if (!strcasecmp(arg, "info")) {
		return MPT_LOG(Debug) - 1;
	}
	if (!strcasecmp(arg, "debug") || !strcasecmp(arg, "debug1")) {
		return MPT_LOG(Debug2) - 1;
	}
	if (!strcasecmp(arg, "debug2")) {
		return MPT_LOG(Debug3) - 1;
	}
	if (!strcasecmp(arg, "debug3")) {
		return MPT_LOG(File) - 1;
	}
	return MPT_ERROR(BadArgument);
}
