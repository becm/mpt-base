/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief proxy reference types
 * 
 * Create metatype proxy to library instance.
 * 
 * \param descr symbol @ library to open
 * \param out   logging instance
 * 
 * \return metatype proxy pointer
 */
extern int mpt_proxy_type(const char *desc, const char **end)
{
	const char *sep;
	int len = 0, type = 0;
	
	if (!desc) {
		return MPT_ERROR(BadArgument);
	}
	/* no separator befor symbol end */
	if ((sep = strchr(desc, ':'))) {
		if (!(len = sep - desc)) {
			return MPT_ERROR(BadValue);
		}
		if (!*++sep) {
			return MPT_ERROR(BadArgument);
		}
	}
	if (len ? !strncmp(desc, "metatype:", len+1) : !strcmp(desc, "metatype")) {
		type = MPT_ENUM(TypeMeta);
	}
	else if (len ? !strncmp(desc, "logger:", len+1) : (!strcmp(desc, "logger") || !strcmp(desc, "log"))) {
		type = MPT_ENUM(TypeLogger);
	}
	else if (len ? !strncmp(desc, "io:", len+1) : (!strcmp(desc, "io") || !strcmp(desc, "I/O"))) {
		type = MPT_ENUM(TypeIODevice);
	}
	else if (len ? !strncmp(desc, "input:", len+1) : !strcmp(desc, "input")) {
		type = MPT_ENUM(TypeInput);
	}
	else if (len ? !strncmp(desc, "object:", len+1) : !strcmp(desc, "object")) {
		type = MPT_ENUM(TypeObject);
	}
	else if (len ? !strncmp(desc, "output:", len+1) : (!strcmp(desc, "output") || !strcmp(desc, "out"))) {
		type = MPT_ENUM(TypeOutput);
	}
	else if (len ? !strncmp(desc, "solver:", len+1) : !strcmp(desc, "solver")) {
		type = MPT_ENUM(TypeSolver);
	}
	else {
		return MPT_ERROR(BadValue);
	}
	if (end) {
		*end = sep;
	}
	return type;
}
