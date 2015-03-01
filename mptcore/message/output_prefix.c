
#include <unistd.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief message prefix
 * 
 * Determine text prefix for message of
 * specified severity for file descriptor.
 * 
 * \param type	type of message
 * \param fd	output file descriptor
 * 
 * \return prefix text
 */
extern const char *mpt_output_prefix(uint8_t type)
{
	if (!type || type >= MPT_ENUM(LogFile)) {
		return 0;
	}
	/*/
	if (type < MPT_ENUM(LogInfo))         return "\033[32m";
	else if (type < MPT_ENUM(LogWarning)) return "\033[34m";
	else if (type < MPT_ENUM(LogError))   return "\033[33m";
	else if (type < MPT_ENUM(LogFatal))   return "\033[35m";
	else if (type < MPT_ENUM(LogFile))    return "\033[31m";*/
	if (type >= MPT_ENUM(LogDebug))    return "\033[32m";
	if (type >= MPT_ENUM(LogInfo))     return "\033[34m";
	if (type >= MPT_ENUM(LogWarning))  return "\033[33m";
	if (type >= MPT_ENUM(LogError))    return "\033[35m";
	return "\033[31m";
}
