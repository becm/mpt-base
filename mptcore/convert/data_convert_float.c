/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <string.h>
#include <float.h>

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
extern int mpt_data_convert_float32(const float *from, int type, void *dest)
{
	float val = 0.0;
	if (from) {
		val = *from;
	}
	switch (type) {
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
		case MPT_type_toVector('f'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
			}
			return sizeof(struct iovec);
		default: return MPT_ERROR(BadType); /* invalid conversion */
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
extern int mpt_data_convert_float64(const double *from, int type, void *dest)
{
	double val = 0.0;
	if (from) {
		val = *from;
	}
	switch (type) {
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
		case MPT_type_toVector('d'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
			}
			return sizeof(struct iovec);
		default: return MPT_ERROR(BadType); /* invalid conversion */
	}
}

#ifdef _MPT_FLOAT_EXTENDED_H
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
extern int mpt_data_convert_exflt(const long double *from, int type, void *dest)
{
	long double val = 0.0L;
	if (from) {
		val = *from;
	}
	switch (type) {
		case 'f':
			if (dest) *((float *) dest) = val;
			return sizeof(float);
		case 'd':
			if (dest) *((double *) dest) = val;
			return sizeof(double);
		case 'e':
			if (dest) *((long double *) dest) = val;
			return sizeof(long double);
		case MPT_type_toVector('e'):
			if (dest) {
				struct iovec *vec = dest;
				vec->iov_base = (void *) from;
				vec->iov_len  = sizeof(*from);
			}
			return sizeof(struct iovec);
		default: return MPT_ERROR(BadType); /* invalid conversion */
	}
}
#endif
