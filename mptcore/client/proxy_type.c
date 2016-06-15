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
extern int mpt_proxy_type(MPT_STRUCT(proxy) *p, const char *desc)
{
	const char *sep;
	int len = 0, type = 0;
	
	/* typed reference */
	if (!desc) {
		return MPT_ERROR(BadArgument);
	}
	/* no separator befor symbol end */
	if ((sep = strchr(desc, ':'))
	    && !(len = sep - desc)) {
		return MPT_ERROR(BadValue);
	}
	if (len ? !strncmp(desc, "metatype:", len+1) : !strcmp(desc, "metatype")) {
		type = MPT_ENUM(TypeMeta);
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
	else if (len ? !strncmp(desc, "solver:", len+1) : !strcmp(desc, "solver")) {
		type = MPT_ENUM(TypeSolver);
	}
	else if (len ? !strncmp(desc, "layout:", len+1) : !strcmp(desc, "layout")) {
		type = MPT_ENUM(TypeGroup);
	}
	else {
		return MPT_ERROR(BadValue);
	}
	memset(&p->_types, 0, sizeof(p->_types));
	
	p->_types[0] = type;
	if (type > MPT_ENUM(TypeObject)
	    && type < MPT_ENUM(TypeVecBase)) {
		p->_types[1] = MPT_ENUM(TypeObject);
	}
	return len ? len + 1 : 0;
}
