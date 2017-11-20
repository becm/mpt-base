/*!
 * read basic and specific configuration from files.
 */

#include <string.h>

#include "meta.h"

#include "object.h"

/*!
 * \ingroup mptObject
 * \brief set object properties
 * 
 * Assign object properties to iterator arguments.
 * 
 * \param obj   object interface
 * \param args  source for properties to set
 * 
 * \return error or offset of first failed element
 */
extern int mpt_object_args(MPT_INTERFACE(object) *obj, MPT_INTERFACE(iterator) *args)
{
	MPT_STRUCT(property) pr;
	int res, count;
	
	do {
		char name[256];
		/* get assign target */
		if ((res = args->_vptr->get(args, MPT_ENUM(TypeProperty), &pr)) < 0) {
			const char *end;
			size_t len;
			res = args->_vptr->get(args, 's', &end);
			if (res < 0) {
				return (res && count) ? count : res;
			}
			if (!(pr.name = end) || !(end = strchr(end, '='))) {
				return count ? count : MPT_ERROR(BadValue);
			}
			len = end - pr.name;
			if (len >= sizeof(name)) {
				return MPT_ERROR(MissingBuffer);
			}
			name[len++] = 0;
			pr.name = memcpy(name, pr.name, len);
			pr.val.fmt = 0;
			pr.val.ptr = end + 1;
		}
		/* only single top level assign */
		else if (!pr.name) {
			return count ? count : MPT_ERROR(BadType);
		}
		if (mpt_object_set_value(obj, pr.name, &pr.val) < 0) {
			return count ? count : MPT_ERROR(BadValue);
		}
		/* assign config */
		++count;
	} while ((res = args->_vptr->advance(args)) > 0);
	
	return res ? count : 0;
}

