/*!
 * print arguments to destination string.
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "object.h"

#include "../mptplot/layout.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief print data
 * 
 * Add stringifyed data to buffer.
 * 
 * \param val  value format and data
 * \param save function to save text data
 * \param dest save target pointer
 * 
 * \return number of processed elements (0 = all)
 */
extern int mpt_tostring(const MPT_STRUCT(value) *val, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	static const MPT_STRUCT(value_format) vfmt = MPT_VALFMT_INIT;
	const void *ptr;
	const char *text;
	size_t len = 0;
	int curr, adv;
	char buf[256];
	
	/* only default domain is supported */
	if (val->domain) {
		return MPT_ERROR(BadType);
	}
	/* data is direct text representation */
	ptr = val->ptr;
	if ((text = mpt_data_tostring(&ptr, val->type, &len))) {
		return save(dest, text, len);
	}
	/* represent color data */
	if (val->type == MPT_ENUM(TypeColor)) {
		const MPT_STRUCT(color) *c;
		if (!(c = val->ptr)) {
			adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x", 0, 0, 0);
		} else if (c->alpha != 0xff) {
			adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c->red, c->green, c->blue, c->alpha);
		} else {
			adv = snprintf(buf, sizeof(buf), "#%02x%02x%02x", c->red, c->green, c->blue);
		}
		if (adv < 0) {
			return adv;
		}
		return save(dest, buf, adv);
	}
	if (val->type == MPT_ENUM(TypeObjectPtr)) {
		const MPT_INTERFACE(object) *obj = *((void * const *) ptr);
		MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
		uintptr_t pos;
		int len, ret, first;
		
		if (!obj) {
			return MPT_ERROR(MissingData);
		}
		/* object content */
		if ((first = save(dest, "{", 1)) < 0) {
			return first;
		}
		pr.name = "";
		if ((ret = obj->_vptr->property(obj, &pr) < 0)) {
			return ret;
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
			if ((ret = mpt_tostring(&pr.val, save, dest)) < 0) {
				return ret;
			}
			len += ret;
		}
		if ((ret = save(dest, " }", 2)) < 0) {
			return ret;
		}
		return first + len + ret;
	}
	/* vector representation in value */
	if ((curr = MPT_type_toScalar(val->type)) > 0) {
		const MPT_STRUCT(type_traits) *traits = mpt_type_traits(curr);
		const struct iovec *vec = val->ptr;
		int total;
		if (!traits || !traits->size) {
			return MPT_ERROR(BadType);
		}
		if ((total = save(dest, "[ ", 2)) < 2) {
			return MPT_ERROR(MissingBuffer);
		}
		if (vec) {
			const uint8_t *data = vec->iov_base;
			size_t i, size = traits->size, max = vec->iov_len / size;
			
			for (i = 0; i < max; i++) {
				int step;
				adv = mpt_number_print(buf, sizeof(buf), vfmt, curr, data);
				if (adv < 0) {
					return adv;
				}
				data += size;
				if ((step = save(dest, buf, adv)) < adv) {
					return MPT_ERROR(MissingBuffer);
				}
				total += step;
				if ((step = save(dest, " ", 1)) < 1) {
					return MPT_ERROR(MissingBuffer);
				}
				total += step;
			}
		}
		if ((adv = save(dest, "]", 1)) < 0) {
			return MPT_ERROR(MissingBuffer);
		}
		return total + adv;
	}
	/* represent numeric value */
	if ((adv = mpt_number_print(buf, sizeof(buf), vfmt, val->type, val->ptr)) >= 0) {
		return save(dest, buf, adv);
	}
	return adv;
}
