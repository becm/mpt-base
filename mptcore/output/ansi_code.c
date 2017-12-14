
#include <unistd.h>

#include "message.h"

/*!
 * \ingroup mptOutput
 * \brief ansi colour code
 * 
 * Determine ANSI colour for log entry of specified severity.
 * 
 * \param type type of log entry
 * 
 * \return ANSI attribute string
 */
extern const char *mpt_ansi_code(uint8_t type)
{
	if (type & MPT_LOG(File)) {
		return 0;
	}
	if (!type) return "\033[1m";
	if (type >= MPT_LOG(Debug))    return "\033[32m";
	if (type >= MPT_LOG(Info))     return "\033[34m";
	if (type >= MPT_LOG(Warning))  return "\033[33m";
	if (type >= MPT_LOG(Error))    return "\033[35m";
	return "\033[31m";
}

/*!
 * \ingroup mptOutput
 * \brief ansi restore code
 * 
 * Get ANSI code to restore default display attributes
 * 
 * \return ANSI reset string
 */
extern const char *mpt_ansi_reset(void)
{
	return "\033[0m";
}
