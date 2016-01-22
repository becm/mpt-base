/*!
 * get logging descriptor in object.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptCore
 * \brief log instance of object
 * 
 * Search log interface in object properties.
 * 
 * \param obj  object descriptor
 * 
 * \return logging descriptor
 */
extern MPT_INTERFACE(logger) *mpt_object_logger(const MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(property) pr = { "", 0, { 0, 0 } };
	void * const *ptr;
	uint8_t type;
	
	if (!obj || obj->_vptr->property(obj, &pr) < 0) {
		return 0;
	}
	if (!pr.val.fmt || !(ptr = pr.val.ptr)) {
		return 0;
	}
	while ((type = *pr.val.fmt++)) {
		if (type == MPT_ENUM(TypeLogger)) {
			return *ptr;
		}
		/* abort on first non-pointer element */
		if ((type < 0x8 || type >= 0x20)
		    && (type < MPT_ENUM(TypeUser) || type >= (MPT_ENUM(TypeUser) + 0x20))) {
			return 0;
		}
		++ptr;
	}
	return 0;
}

