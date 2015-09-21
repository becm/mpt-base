
#include <unistd.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief message type colour
 * 
 * Determine ANSI colour for message of specified severity.
 * 
 * \param type type of message
 * 
 * \return ANSI display attribute string
 */
extern const char *mpt_ansi_code(uint8_t type)
{
	if (!type || type >= MPT_ENUM(LogFile)) {
		return 0;
	}
	if (type >= MPT_ENUM(LogDebug))    return "\033[32m";
	if (type >= MPT_ENUM(LogInfo))     return "\033[34m";
	if (type >= MPT_ENUM(LogWarning))  return "\033[33m";
	if (type >= MPT_ENUM(LogError))    return "\033[35m";
	return "\033[31m";
}

/*!
 * \ingroup mptMessage
 * \brief display restore code
 * 
 * Get ANSI code to restore default display attributes
 * 
 * \return ANSI display attribute string
 */
extern const char *mpt_ansi_restore(void)
{
	return "\033[0m";
}
