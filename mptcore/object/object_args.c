/*!
 * read basic and specific configuration from files.
 */

#include <string.h>

#include "meta.h"
#include "types.h"

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
	
	count = 0;
	while ((res = args->_vptr->get(args, MPT_ENUM(TypeProperty), &pr))) {
		char name[256];
		/* use string property */
		if (res < 0) {
			const char *end;
			
			/* string on iterator */
			if ((res = args->_vptr->get(args, 's', &end)) < 0) {
				return count ? count : res;
			}
			else if (!res) {
				return count;
			}
			/* split property name */
			if (!(pr.name = end)) {
				return count ? count : MPT_ERROR(BadValue);
			}
			/* set non-assign options to default value */
			if (!(end = strchr(end, '='))) {
				MPT_value_set_string(&pr.val, 0);
			} else {
				size_t len;
				MPT_value_set_string(&pr.val, end + 1);
				len = end - pr.name;
				if (len >= sizeof(name)) {
					return count ? count : MPT_ERROR(MissingBuffer);
				}
				name[len++] = 0;
				pr.name = memcpy(name, pr.name, len);
			}
		}
		/* no top level assign */
		else if (!pr.name) {
			return count ? count : MPT_ERROR(BadType);
		}
		if (mpt_object_set_value(obj, pr.name, &pr.val) < 0) {
			return count ? count : MPT_ERROR(BadValue);
		}
		/* assign config */
		++count;
		if ((res = args->_vptr->advance(args)) < 0) {
			return count;
		}
	};
	return count;
}

