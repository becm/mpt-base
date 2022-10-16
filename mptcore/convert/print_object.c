/*!
 * print arguments to destination string.
 */

#include <sys/uio.h>
#include <string.h>

#include "types.h"
#include "object.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief print object data
 * 
 * Add stringifyed data of object to output.
 * 
 * For per-property processing use mpt_properties_print().
 * 
 * \param obj  object to be printed
 * \param save function to amend text data
 * \param dest output context
 * 
 * \return number of written bytes
 */
extern int mpt_print_object(const MPT_INTERFACE(object) *obj, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	uintptr_t pos;
	int len, ret, first;
	
	pr.name = "";
	if ((ret = obj->_vptr->property(obj, &pr) < 0)) {
		return ret;
	}
	/* object content */
	if ((first = save(dest, "{", 1)) < 0) {
		return first;
	}
	len = 0;
	/* nested type info */
	if (pr.name && strcmp(pr.name, "object")) {
		if ((ret = save(dest, " _=\"", 4)) < 0) {
			return ret;
		}
		len += ret;
		if ((ret = save(dest, pr.name, strlen(pr.name))) < 0) {
			return ret;
		}
		len += ret;
		if ((ret = save(dest, "\"", 1)) < 0) {
			return ret;
		}
		len += ret;
	}
	/* properties queried by index */
	pos = 0;
	while (1) {
		static const char sep[] = ", ";
		int seplen = len ? 2 : 1;
		
		pr.name = 0;
		pr.desc = (void *) pos++;
		if ((ret = obj->_vptr->property(obj, &pr) < 0)) {
			break;
		}
		if (!pr.name) {
			continue;
		}
		if ((ret = save(dest, sep + 2 - seplen, seplen)) < 0) {
			return ret;
		}
		len += ret;
		if ((ret = save(dest, pr.name, strlen(pr.name))) < 0) {
			return ret;
		}
		len += ret;
		if ((ret = save(dest, "=", 1)) < 0) {
			return ret;
		}
		len += ret;
		if ((ret = mpt_print_value(&pr.val, save, dest)) < 0) {
			return ret;
		}
		len += ret;
	}
	if ((ret = save(dest, " }", 2)) < 0) {
		return ret;
	}
	return first + len + ret;
}
