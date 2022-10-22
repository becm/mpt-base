/*!
 * read basic and specific configuration from files.
 */

#include <string.h>

#include "types.h"
#include "convert.h"

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
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	const MPT_STRUCT(value) *val;
	int count;
	
	count = 0;
	while ((val = args->_vptr->value(args))) {
		const void *ptr;
		int res;
		
		if (!val) {
			break;
		}
		pr.name = 0;
		pr.val._type = 0;
		ptr = val->_addr;
		if (MPT_type_isConvertable(val->_type)) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) ptr);
			
			if (conv->_vptr->convert(conv, MPT_ENUM(TypeProperty), &pr) < 0
			 || mpt_object_set_value(obj, pr.name, &pr.val) < 0) {
				pr.val._type = 0;
				conv->_vptr->convert(conv, 's', &pr.name);
			}
		}
		/* value is not assigned via property */
		if (!pr.val._type) {
			char name[256];
			const char *str;
			if (!pr.name && !(pr.name = mpt_data_tostring(&ptr, val->_type, 0))) {
				return count ? count : MPT_ERROR(BadValue);
			}
			/* set non-assign options to default value */
			if (!(str = strchr(str, '='))) {
				MPT_property_set_string(&pr, 0);
			} else {
				size_t len;
				MPT_property_set_string(&pr, str + 1);
				len = str - pr.name;
				if (len >= sizeof(name)) {
					return count ? count : MPT_ERROR(MissingBuffer);
				}
				name[len++] = 0;
				pr.name = memcpy(name, pr.name, len);
			}
			/* assign config */
			if (mpt_object_set_value(obj, pr.name, &pr.val) < 0) {
				return count ? count : MPT_ERROR(BadValue);
			}
		}
		if ((res = args->_vptr->advance(args)) <= 0) {
			return count ? res : count;
		}
		++count;
	}
	return count;
}

