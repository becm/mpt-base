/*!
 * line end options.
 */

#include <stddef.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief newline code
 * 
 * Get line separator for build platform.
 * 
 * \return native line separator identifier
 */
extern int mpt_newline_native(void)
{
#if defined(__APPLE__)
	return MPT_ENUM(NewlineMac);
#elif defined(__unix__) || defined(__unix)
	return MPT_ENUM(NewlineUnix);
#elif defined(_WIN32) || defined(_WIN64)
	return MPT_ENUM(NewlineNet);
#else
# error: no native platform line separator
#endif
}

/*!
 * \ingroup mptConvert
 * \brief newline string
 * 
 * Get correct line separator for platform.
 * 
 * \param type  line separator identifier
 * 
 * \return line separator
 */
extern const char *mpt_newline_string(int type)
{
	static const char nl_unix[] = { '\n', 0 };
	static const char nl_mac[] = { '\r', 0 };
	static const char nl_net[] = { '\r', '\n', 0 };
	const char *nl = nl_net;
	
	if (type < 0 || !type) {
		type = mpt_newline_native();
	}
	if (type == MPT_ENUM(NewlineUnix)) {
		nl = nl_unix;
	}
	if (type == MPT_ENUM(NewlineMac)) {
		nl = nl_mac;
	}
	return nl;
}
