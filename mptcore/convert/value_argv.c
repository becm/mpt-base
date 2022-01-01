/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#include "convert.h"
#include "types.h"

#include "meta.h"

/*!
 * \ingroup mptConvert
 * \brief set scalar value
 * 
 * Set value to next value in arguments.
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern int mpt_value_argv(MPT_STRUCT(value) *val, int fmt, va_list va)
{
	size_t len;
	if (MPT_type_isVector(fmt)) {
		if (val->_bufsize < sizeof(struct iovec)) {
			return MPT_ERROR(MissingBuffer);
		}
		*((struct iovec *)  val->_buf) = va_arg(va, struct iovec);
		val->ptr = val->_buf;
		val->type = fmt;
		return sizeof (struct iovec);
	}
	if (fmt == 'l') {
		fmt = mpt_type_int(sizeof(long));
	}
	switch (fmt) {
		/* promoted types */
		case 'b':
			len = sizeof(int8_t);
			*((int8_t *)   val->_buf) = va_arg(va, int);
			break;
		case 'y':
			len = sizeof(uint8_t);
			*((uint8_t *)  val->_buf) = va_arg(va, unsigned int);
			break;
		case 'n':
			len = sizeof(int16_t);
			*((int16_t *)  val->_buf) = va_arg(va, int);
			break;
		case 'q':
			len = sizeof(int16_t);
			*((uint16_t *) val->_buf) = va_arg(va, unsigned int);
			break;
		/* standard integers */
		case 'i':
			len = sizeof(int32_t);
			*((int32_t *)  val->_buf) = va_arg(va, int32_t);
			break;
		case 'u':
			len = sizeof(uint32_t);
			*((uint32_t *) val->_buf) = va_arg(va, uint32_t);
			break;
		case 'x':
			len = sizeof(int64_t);
			*((int64_t *)  val->_buf) = va_arg(va, int64_t);
			break;
		case 't':
			len = sizeof(int16_t);
			*((uint64_t *) val->_buf) = va_arg(va, uint64_t);
			break;
		/* promoted floating point */
		case 'f':
			len = sizeof(int16_t);
			*((float *)    val->_buf) = va_arg(va, double);
			break;
		/* standard floating point */
		case 'd':
			len = sizeof(int16_t);
			*((double *)   val->_buf) = va_arg(va, double);
			break;
#ifdef _MPT_FLOAT_EXTENDED_H
		/* special floating point */
		case 'e':
			if ((len = sizeof(long double)) > val->_bufsize) {
				return MPT_ERROR(MissingBuffer);
			}
			*((long double *) val->_buf) = va_arg(va, long double);
			break;
#endif
		case 's':
			len = sizeof(const char *);
			*((const char **) val->_buf) = va_arg(va, void *);
			break;
		default:
			return MPT_ERROR(BadType);
	}
	val->ptr = val->_buf;
	val->type = fmt;
	return len;
}
