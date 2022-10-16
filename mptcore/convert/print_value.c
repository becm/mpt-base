/*!
 * print arguments to destination string.
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"
#include "object.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief print value data
 * 
 * Add stringifyed data of convertable to output.
 * 
 * \param val  value with number type to be printed
 * \param save function to amend text data
 * \param dest output context
 * 
 * \return number of written bytes
 */
extern int mpt_print_value(const MPT_STRUCT(value) *val, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	static const MPT_STRUCT(value_format) vfmt = MPT_VALFMT_INIT;
	const void *ptr;
	const char *str;
	size_t len;
	int type, curr, adv;
	char buf[256];
	
	/* only default namespace is supported */
	if (val->_namespace) {
		return MPT_ERROR(BadArgument);
	}
	/* undefined type */
	if (!(type = val->type)) {
		return MPT_ERROR(BadType);
	}
	/* missing content */
	if (!(ptr = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	/* object type */
	if (type == MPT_ENUM(TypeObjectPtr)) {
		const MPT_INTERFACE(object) *obj = *(const void **) ptr;
		return obj ? mpt_print_object(obj, save, dest) : 0;
	}
	/* convertable type */
	if (type == MPT_ENUM(TypeConvertablePtr)) {
		MPT_INTERFACE(convertable) *conv = *(void **) ptr;
		return conv ? mpt_print_convertable(conv, save, dest) : 0;
	}
	if ((str = mpt_data_tostring(&ptr, type, &len))) {
		return save(dest, str, len);
	}
	/* vector representation in value */
	if ((curr = MPT_type_toScalar(type)) > 0) {
		const MPT_STRUCT(type_traits) *traits = mpt_type_traits(curr);
		const struct iovec *vec = ptr;
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
				const char *str;
				int step;
				if (curr == 's') {
					if ((str = *((const char **) data))) {
						adv = strlen(str);
					}
					else {
						adv = 0;
						str = "";
					}
				}
				else {
					MPT_STRUCT(value) pos = MPT_VALUE_INIT(curr, data);
					adv = mpt_number_tostring(&pos, vfmt, buf, sizeof(buf));
					if (adv < 0) {
						return adv;
					}
					str = buf;
				}
				if ((step = save(dest, str, adv)) < adv) {
					return MPT_ERROR(MissingBuffer);
				}
				total += step;
				if ((step = save(dest, " ", 1)) < 1) {
					return MPT_ERROR(MissingBuffer);
				}
				total += step;
				data  += size;
			}
		}
		if ((adv = save(dest, "]", 1)) < 0) {
			return MPT_ERROR(MissingBuffer);
		}
		return total + adv;
	}
	/* represent numeric value */
	if ((adv = mpt_number_tostring(val, vfmt, buf, sizeof(buf))) >= 0) {
		return save(dest, buf, adv);
	}
	return adv;
}
