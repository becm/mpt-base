/*!
 * print arguments to destination string.
 */

#include <sys/uio.h>

#include "types.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief print convertable data
 * 
 * Add stringifyed data of convertable to output.
 * 
 * \param conv convertable to be printed
 * \param save function to amend text data
 * \param dest output context
 * 
 * \return number of written bytes
 */
extern int mpt_print_convertable(MPT_INTERFACE(convertable) *conv, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	struct iovec vec;
	int type;
	
	/* only default namespace is supported */
	if ((type = conv->_vptr->convert(conv, MPT_type_toVector('c'), &vec)) >= 0) {
		return save(dest, vec.iov_base, vec.iov_len);
	}
	/* get default type of convertable */
	if ((type = conv->_vptr->convert(conv, 0, 0)) < 0) {
		return type;
	}
	/* only core value types are supported */
	if (type < MPT_ENUM(TypeConvertablePtr)) {
		MPT_STRUCT(value) val;
		uint8_t buf[256];
		int ret;
		/* copy data to buffer */
		if ((ret = conv->_vptr->convert(conv, type, buf)) < 0) {
			return ret;
		}
		MPT_value_set(&val, type, buf);
		return mpt_print_value(&val, save, dest);
	}
	return MPT_ERROR(BadType);
}
