/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <string.h>
#include <ctype.h>

#include <sys/uio.h>

#include "types.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_int8(const int8_t *from, MPT_TYPE(type) type, void *dest)
{
	int8_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'y':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'b':
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'q':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'n':
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'u':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'i':
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 't':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
	#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
	#endif
		case MPT_type_toVector('b'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
	return sizeof(*from);
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_uint8(const uint8_t *from, MPT_TYPE(type) type, void *dest)
{
	uint8_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val > INT8_MAX) return MPT_ERROR(BadValue);
			/* fall through */
		case 'y':
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'n':
		case 'q':
			if (dest) *((uint16_t *) dest) = val;
			return sizeof(uint16_t);
		case 'i':
		case 'u':
			*((uint32_t *) dest) = val;
			return sizeof(uint32_t);
		case 'x':
		case 't':
			*((uint64_t *) dest) = val;
			return sizeof(uint64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
	#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(double);
	#endif
		case MPT_type_toVector('y'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_int16(const int16_t *from, MPT_TYPE(type) type, void *dest)
{
	int16_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b': if (val < INT8_MIN || val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'y': if (val < 0 || val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'q':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'n':
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'u':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'i':
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 't':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(int64_t);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
	#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
	#endif
		case MPT_type_toVector('n'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_uint16(const uint16_t *from, MPT_TYPE(type) type, void *dest)
{
	uint16_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(char);
		case 'y':
			if (val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'h':
			if (val > INT16_MAX) return MPT_ERROR(BadValue);
			/* fall through */
		case 'q':
			if (dest) *((uint16_t *) dest) = val;
			return sizeof(uint16_t);
		case 'u':
		case 'i':
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 't':
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e': *((long double *) dest) = val;
			return sizeof(long double);
#endif
		case MPT_type_toVector('q'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
	return sizeof(*from);
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_int32(const int32_t *from, MPT_TYPE(type) type, void *dest)
{
	int32_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val < INT8_MIN || val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'y':
			if (val < 0 || val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'n': if (val < INT16_MIN || val > INT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'q':
			if (val < 0 || val > UINT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint16_t *) dest) = val;
			return sizeof(uint16_t);
		case 'u':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'i':
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 't':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
#endif
		case MPT_type_toVector('i'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_uint32(const uint32_t *from, MPT_TYPE(type) type, void *dest)
{
	uint32_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'y':
			if (val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'n':
			if (val > INT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'q':
			if (val > UINT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'i':
			if (val > INT32_MAX) return MPT_ERROR(BadValue);
			/* fall through */
		case 'u':
			if (dest) *((uint32_t *) dest) = val;
			return sizeof(uint32_t);
		case 't':
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
#endif
		case MPT_type_toVector('u'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_int64(const int64_t *from, MPT_TYPE(type) type, void *dest)
{
	int64_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val < INT8_MIN || val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'y':
			if (val < 0 || val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'n':
			if (val < INT16_MIN || val > INT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'q':
			if (val < 0 || val > UINT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint16_t *) dest) = val;
			return sizeof(uint16_t);
		case 'i':
			if (val < INT32_MIN || val > INT32_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 'u':
			if (val < 0 || val > UINT32_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint32_t *) dest) = val;
			return sizeof(uint32_t);
		case 't':
			if (val < 0) return MPT_ERROR(BadValue);
			/* fall through */
		case 'x':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
#endif
		case MPT_type_toVector('x'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param from  source data
 * \param type source data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */
extern int mpt_data_convert_uint64(const uint64_t *from, MPT_TYPE(type) type, void *dest)
{
	uint64_t val = 0;
	if (from) {
		val = *from;
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
		case 'c':
			if (!isgraph(val)) return MPT_ERROR(BadValue);
			if (dest) *((char *) dest) = val;
			return sizeof(char);
		case 'b':
			if (val > INT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int8_t *) dest) = val;
			return sizeof(int8_t);
		case 'y':
			if (val > UINT8_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint8_t *) dest) = val;
			return sizeof(uint8_t);
		case 'n':
			if (val > INT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int16_t *) dest) = val;
			return sizeof(int16_t);
		case 'q':
			if (val > UINT16_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint16_t *) dest) = val;
			return sizeof(uint16_t);
		case 'i':
			if (val > INT32_MAX) return MPT_ERROR(BadValue);
			if (dest) *((int32_t *) dest) = val;
			return sizeof(int32_t);
		case 'u':
			if (val > UINT32_MAX) return MPT_ERROR(BadValue);
			if (dest) *((uint32_t *) dest) = val;
			return sizeof(uint32_t);
		case 'x':
			if (val > INT64_MAX) return MPT_ERROR(BadValue);
			/* fall through */
		case 't':
			if (dest) *((int64_t *) dest) = val;
			return sizeof(int64_t);
		
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
#endif
		case MPT_type_toVector('t'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
				return sizeof(*vec);
			}
			return MPT_ERROR(MissingData);
		default:
			/* invalid conversion */
			return MPT_ERROR(BadType);
	}
}
