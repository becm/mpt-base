/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#include <sys/uio.h>

#include "types.h"

/*!
 * \ingroup mptConvert
 * \brief set scalar value
 * 
 * Set value to next value in arguments.
 * 
 * \param vec target data buffer
 * \param fmt next element type in arguments
 * \param va  variadic data
 */
extern int mpt_value_argv(const struct iovec *vec, int fmt, va_list va)
{
	size_t len;
	if (MPT_type_isVector(fmt)) {
		if (vec->iov_len < sizeof(struct iovec)) {
			return MPT_ERROR(MissingBuffer);
		}
		*((struct iovec *)  vec->iov_base) = va_arg(va, struct iovec);
		return sizeof (struct iovec);
	}
	if (fmt == 'l') {
		fmt = mpt_type_int(sizeof(long));
	}
	switch (fmt) {
		/* promoted types */
		case 'b':
			len = sizeof(int8_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((int8_t *)   vec->iov_base) = va_arg(va, int);
			return len;
		case 'y':
			len = sizeof(uint8_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((uint8_t *)  vec->iov_base) = va_arg(va, unsigned int);
			return len;
		case 'n':
			len = sizeof(int16_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((int16_t *)  vec->iov_base) = va_arg(va, int);
			return len;
		case 'q':
			len = sizeof(int16_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((uint16_t *) vec->iov_base) = va_arg(va, unsigned int);
			return len;
		/* standard integers */
		case 'i':
			len = sizeof(int32_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((int32_t *)  vec->iov_base) = va_arg(va, int32_t);
			return len;
		case 'u':
			len = sizeof(uint32_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((uint32_t *) vec->iov_base) = va_arg(va, uint32_t);
			return len;
		case 'x':
			len = sizeof(int64_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((int64_t *)  vec->iov_base) = va_arg(va, int64_t);
			return len;
		case 't':
			len = sizeof(int64_t);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((uint64_t *) vec->iov_base) = va_arg(va, uint64_t);
			return len;
		/* promoted floating point */
		case 'f':
			len = sizeof(float);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((float *)    vec->iov_base) = va_arg(va, double);
			return len;
		/* standard floating point */
		case 'd':
			len = sizeof(double);
			if (len > vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((double *)   vec->iov_base) = va_arg(va, double);
			return len;
#ifdef _MPT_FLOAT_EXTENDED_H
		/* special floating point */
		case 'e':
			len = sizeof(long double);
			if (len < vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((long double *) vec->iov_base) = va_arg(va, long double);
			return len;
#endif
		case 's':
			len = sizeof(const char *);
			if (len < vec->iov_len) {
				return MPT_ERROR(MissingBuffer);
			}
			*((const char **) vec->iov_base) = va_arg(va, void *);
			return len;
		default:
			return MPT_ERROR(BadType);
	}
}
