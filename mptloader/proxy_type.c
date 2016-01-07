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
	int len, type = 0;
	
	/* typed reference */
	if (!desc) {
		return MPT_ERROR(BadArgument);
	}
	/* no separator befor symbol end */
	if (!(sep = strchr(desc, ':'))) {
		len = -1;
	}
	else if (memchr(desc, '@', len = sep - desc)) {
		return 0;
	}
	if (len < 0 ? !strcmp(desc, "metatype") : !strncmp(desc, "metatype:", len+1)) {
		type = MPT_ENUM(TypeMeta);
	}
	else if (len < 0 ? !strcmp(desc, "object") : !strncmp(desc, "object:", len+1)) {
		type = MPT_ENUM(TypeObject);
	}
	else if (len < 0 ? !strcmp(desc, "solver") : !strncmp(desc, "solver:", len+1)) {
		type = MPT_ENUM(TypeSolver);
	}
	else if (len < 0 ? !strcmp(desc, "input") : !strncmp(desc, "input:", len+1)) {
		type = MPT_ENUM(TypeInput);
	}
	else {
		return MPT_ERROR(BadValue);
	}
	memset(&p->_types, 0, sizeof(p->_types));
	switch (type) {
	  case MPT_ENUM(TypeMeta):
		*p->_types = MPT_ENUM(TypeMeta);
		break;
	  case MPT_ENUM(TypeObject):
		*p->_types = MPT_ENUM(TypeObject);
		break;
	  case MPT_ENUM(TypeInput):
		*p->_types = MPT_ENUM(TypeInput);
		break;
	  case MPT_ENUM(TypeSolver):
		p->_types[0] = MPT_ENUM(TypeSolver);
		p->_types[1] = MPT_ENUM(TypeObject);
		break;
	  case MPT_ENUM(TypeOutput):
		p->_types[0] = MPT_ENUM(TypeOutput);
		p->_types[1] = MPT_ENUM(TypeObject);
		break;
	  default:;
	}
	return len + 1;
}
