/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#include <sys/uio.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief set scalar value
 * 
 * Set value to next value in arguments.
 * 
 * \param vec target data buffer
 * \param fmt next element type in arguments
 * \param va  variadic data
 */
extern int mpt_value_argv(void *dest, size_t max, int fmt, va_list va)
{
	size_t len;
	if (MPT_type_isVector(fmt)) {
		if ((len = sizeof(struct iovec)) > max) {
			return MPT_ERROR(MissingBuffer);
		}
		if (dest) *((struct iovec *) dest) = va_arg(va, struct iovec);
		return len;
	}
	if (fmt == 'l') {
		fmt = mpt_type_int(sizeof(long));
	}
	switch (fmt) {
		/* promoted types */
		case 'b':
			if ((len = sizeof(int8_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((int8_t *)   dest) = va_arg(va, int);
			return len;
		case 'y':
			if ((len = sizeof(uint8_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((uint8_t *)  dest) = va_arg(va, unsigned int);
			return len;
		case 'n':
			if ((len = sizeof(int16_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((int16_t *)  dest) = va_arg(va, int);
			return len;
		case 'q':
			if ((len = sizeof(int16_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((uint16_t *) dest) = va_arg(va, unsigned int);
			return len;
		/* standard integers */
		case 'i':
			if ((len = sizeof(int32_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((int32_t *)  dest) = va_arg(va, int32_t);
			return len;
		case 'u':
			if ((len = sizeof(uint32_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((uint32_t *) dest) = va_arg(va, uint32_t);
			return len;
		case 'x':
			if ((len = sizeof(int64_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((int64_t *)  dest) = va_arg(va, int64_t);
			return len;
		case 't':
			if ((len = sizeof(int64_t)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((uint64_t *) dest) = va_arg(va, uint64_t);
			return len;
		/* promoted floating point */
		case 'f':
			if ((len = sizeof(float)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((float *)    dest) = va_arg(va, double);
			return len;
		/* standard floating point */
		case 'd':
			if ((len = sizeof(double)) > max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((double *)   dest) = va_arg(va, double);
			return len;
#ifdef _MPT_FLOAT_EXTENDED_H
		/* special floating point */
		case 'e':
			if ((len = sizeof(long double)) < max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((long double *) dest) = va_arg(va, long double);
			return len;
#endif
		case 's':
			if ((len = sizeof(const char *)) < max) {
				return MPT_ERROR(MissingBuffer);
			}
			if (dest) *((const char **) dest) = va_arg(va, void *);
			return len;
		default:
			return MPT_ERROR(BadType);
	}
}
