/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <ctype.h>
#include <string.h>
#include <float.h>

#include <sys/uio.h>

#include "array.h"
#include "object.h"

#include "meta.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param fptr  source data
 * \param ftype source data type
 * \param dest  destination pointer
 * \param dtype destination data type
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */

extern int mpt_data_convert(const void **fptr, int ftype, void *dest, int dtype)
{
	const uint8_t *from = *fptr;
	int flen, dlen;
	
	/* special array processing */
	if (ftype == MPT_ENUM(TypeArray)) {
		const MPT_STRUCT(array) *arr = (void *) from;
		MPT_STRUCT(buffer) *b;
		int content = 0;
		
		/* special content on buffer */
		if ((b = arr->_buf)) {
			const MPT_STRUCT(type_traits) *info = 0;
			content = -1;
			if ((info = b->_typeinfo)
			    && (!(content = info->type))) {
				return MPT_ERROR(BadValue);
			}
		}
		if (ftype == dtype) {
			if (dest
			    && mpt_array_clone(dest, b ? arr : 0) < 0) {
				return MPT_ERROR(BadOperation);
			}
			*fptr = arr + 1;
			return sizeof(*arr);
		}
		if (dtype == 's') {
			const char **txt = dest;
			const char *s;
			
			if (content && content != 'c') {
				return MPT_ERROR(BadType);
			}
			if (b && !(s = memchr(b + 1, 0, b->_used))) {
				return MPT_ERROR(BadValue);
			}
			*fptr = arr + 1;
			
			if (dest) *txt = b ? (void *) (b + 1) : 0;
			return sizeof(*txt);
		}
		if ((dtype = MPT_type_vector(dtype)) >= 0) {
			struct iovec *vec;
			/* require matching types */
			if (dtype != content) {
				return MPT_ERROR(BadType);
			}
			if ((vec = dest)) {
				if (b) {
					vec->iov_base = b + 1;
					vec->iov_len = b->_used;
				} else {
					vec->iov_base = 0;
					vec->iov_len = 0;
				}
			}
			*fptr = arr + 1;
			return sizeof(*vec);
		}
		return MPT_ERROR(BadType);
	}
	/* metatype pointer processing */
	if (MPT_type_isMetaPtr(ftype)) {
		MPT_INTERFACE(metatype) *mt = *((MPT_INTERFACE(metatype) * const *) from);
		flen = sizeof(void *);
		if (dtype == ftype
		 || dtype == MPT_ENUM(TypeMetaPtr)) {
			if (dest) {
				*((void **) dest) = *((void **) from);
			}
			*fptr = from + sizeof(void *);
			return flen;
		}
		if (!mt) {
			return MPT_ERROR(BadOperation);
		}
		if ((flen = mpt_valsize(dtype)) < 0) {
			return flen;
		}
		if (mt->_vptr->conv(mt, dtype, dest) <= 0) {
			return MPT_ERROR(BadOperation);
		}
		*fptr = from + sizeof(void *);
		return flen;
	}
	/* special metatype processing */
	if (ftype == MPT_ENUM(TypeMetaRef)
	 || MPT_type_isMetaRef(ftype)) {
		MPT_INTERFACE(metatype) **ptr;
		MPT_INTERFACE(metatype) *mt = *((MPT_INTERFACE(metatype) * const *) from);
		
		flen = sizeof(mt);
		
		/* require valid reference for target */
		if (dtype != MPT_ENUM(TypeMetaRef)
		 && dtype != ftype) {
			return MPT_ERROR(BadType);
		}
		if ((ptr = dest)) {
			MPT_INTERFACE(metatype) *old;
			
			if (mt && !mt->_vptr->instance.addref((void *) mt)) {
				return MPT_ERROR(BadOperation);
			}
			if ((old = *ptr)) {
				old->_vptr->instance.addref((void *) old);
			}
			*ptr = mt;
		}
		*fptr = from + flen;
		return sizeof(*ptr);
	}
	/* check input type size */
	if ((flen = mpt_valsize(ftype)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* pointer type */
	else if (!flen) {
		flen = sizeof(void *);
	}
	/* same type (copy pointer) */
	if (ftype == dtype) {
		if (dest) {
			memcpy(dest, from, flen);
		}
		*fptr = from + flen;
		return flen;
	}
	/* check output type size */
	if ((dlen = mpt_valsize(dtype)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* advance source only */
	if (!dest) {
		*fptr = from + flen;
		return dlen ? dlen : (int) sizeof(void *);
	}
	if (ftype == 'l') {
		ftype = mpt_type_int(sizeof(long));
	}
	if (dtype == 'l') {
		dtype = mpt_type_int(sizeof(long));
	}
	/* number processing */
	switch (ftype) {
	  case 'c':
		if (dtype == 'c') {
			*((char *) dest) = *((char *) from);
			break;
		}
	  case 'b':
		switch (dtype) {
		  case 'c': if (!isgraph(*((int8_t *) from))) return MPT_ERROR(BadValue);
		  case 'y': if (*((int8_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'b': *((int8_t *) dest) = *((int8_t *) from); break;
		  case 'q': if (*((int8_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'n': *((int16_t *) dest) = *((int8_t *) from); break;
		  case 'u': if (*((int8_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'i': *((int32_t *) dest) = *((int8_t *) from); break;
		  case 't': if (*((int8_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'x': *((int32_t *) dest) = *((int8_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((int8_t *) from); break;
		  case 'd': *((double *) dest) = *((int8_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((int8_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'y':
		switch (dtype) {
		  case 'c': if (!isgraph(*((int8_t *) from))) return MPT_ERROR(BadValue);
		            *((char *) dest)   = *((uint8_t *) from); break;
		  case 'b': if (*((uint8_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest)   = *((uint8_t *) from); break;
		  case 'y': *((uint8_t *) dest)  = *((uint8_t *) from); break;
		  case 'n':
		  case 'q': *((uint16_t *) dest) = *((uint8_t *) from); break;
		  case 'i':
		  case 'u': *((uint32_t *) dest) = *((uint8_t *) from); break;
		  case 'x':
		  case 't': *((uint64_t *) dest) = *((uint8_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((uint8_t *) from); break;
		  case 'd': *((double *) dest) = *((uint8_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((uint8_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'n':
		switch (dtype) {
		  case 'c': if (!isgraph(*((int16_t *) from))) return MPT_ERROR(BadValue);
		           *((char *) dest) = *((int16_t *) from); break;
		  case 'b': if (*((int16_t *) from) < INT8_MIN || *((int16_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest) = *((int16_t *) from); break;
		  case 'y': if (*((int16_t *) from) < 0 || *((int16_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
		            *((uint8_t *) dest) = *((int16_t *) from); break;
		  case 'q': if (*((int16_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'n': *((int16_t *) dest) = *((int16_t *) from); break;
		  case 'u': if (*((int16_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'i': *((int32_t *) dest) = *((int16_t *) from); break;
		  case 't': if (*((int16_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'x': *((int64_t *) dest) = *((int16_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((int16_t *) from); break;
		  case 'd': *((double *) dest) = *((int16_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((int16_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'q':
		switch (dtype) {
		  case 'c': if (!isgraph(*((uint16_t *) from))) return MPT_ERROR(BadValue);
		           *((char *) dest) = *((uint16_t *) from); break;
		  case 'b': if (*((uint16_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
			    *((int8_t *) dest) = *((uint16_t *) from); break;
		  case 'y': if (*((uint16_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
			    *((uint8_t *) dest) = *((uint16_t *) from); break;
		  case 'h': if (*((uint16_t *) from) > INT16_MAX) return MPT_ERROR(BadValue);
		  case 'q': *((uint16_t *) dest) = *((uint16_t *) from); break;
		  case 'u':
		  case 'i': *((int32_t *) dest) = *((uint16_t *) from); break;
		  case 't':
		  case 'x': *((int64_t *) dest) = *((uint16_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((uint16_t *) from); break;
		  case 'd': *((double *) dest) = *((uint16_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((uint16_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'i':
		switch (dtype) {
		  case 'c': if (!isgraph(*((int32_t *) from))) return MPT_ERROR(BadValue);
		  case 'b': if (*((int32_t *) from) < INT8_MIN || *((int32_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest) = *((int32_t *) from); break;
		  case 'y': if (*((int32_t *) from) < 0 || *((int32_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
		            *((uint8_t *) dest) = *((int32_t *) from); break;
		  case 'n': if (*((int32_t *) from) < INT16_MIN || *((int32_t *) from) > INT16_MAX) return MPT_ERROR(BadValue);
		            *((int16_t *) dest) = *((int32_t *) from); break;
		  case 'q': if (*((int32_t *) from) < 0 || *((int32_t *) from) > UINT16_MAX) return MPT_ERROR(BadValue);
		            *((uint16_t *) dest) = *((int32_t *) from); break;
		  case 'u': if (*((int32_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'i': *((int32_t *) dest) = *((int32_t *) from); break;
		  case 't': if (*((int32_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'x': *((int64_t *) dest) = *((int32_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((int32_t *) from); break;
		  case 'd': *((double *) dest) = *((int32_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((int32_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'u':
		switch (dtype) {
		  case 'c': if (!isgraph(*((uint32_t *) from))) return MPT_ERROR(BadValue);
		            *((char *) dest) = *((uint32_t *) from); break;
		  case 'b': if (*((uint32_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest) = *((uint32_t *) from); break;
		  case 'y': if (*((uint32_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
		            *((uint8_t *) dest) = *((uint32_t *) from); break;
		  case 'n': if (*((uint32_t *) from) > INT16_MAX) return MPT_ERROR(BadValue);
		            *((int16_t *) dest) = *((uint32_t *) from); break;
		  case 'q': if (*((uint32_t *) from) > UINT16_MAX) return MPT_ERROR(BadValue);
		            *((int16_t *) dest) = *((uint32_t *) from); break;
		  case 'i': if (*((uint32_t *) from) > INT32_MAX) return MPT_ERROR(BadValue);
		  case 'u': *((uint32_t *) dest) = *((uint32_t *) from); break;
		  case 't':
		  case 'x': *((int64_t *) dest) = *((uint32_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((uint32_t *) from); break;
		  case 'd': *((double *) dest) = *((uint32_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((uint32_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'x':
		switch (dtype) {
		  case 'c': if (!isgraph(*((int64_t *) from))) return MPT_ERROR(BadValue);
		            *((char *) dest) = *((int64_t *) from); break;
		  case 'b': if (*((int64_t *) from) < INT8_MIN || *((int64_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest) = *((int64_t *) from); break;
		  case 'y': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
		            *((uint8_t *) dest) = *((int64_t *) from); break;
		  case 'n': if (*((int64_t *) from) < INT16_MIN || *((int64_t *) from) > INT16_MAX) return MPT_ERROR(BadValue);
		            *((int16_t *) dest) = *((int64_t *) from); break;
		  case 'q': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT16_MAX) return MPT_ERROR(BadValue);
		            *((uint16_t *) dest) = *((int64_t *) from); break;
		  case 'i': if (*((int64_t *) from) < INT32_MIN || *((int64_t *) from) > INT32_MAX) return MPT_ERROR(BadValue);
		            *((int32_t *) dest) = *((int64_t *) from); break;
		  case 'u': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT32_MAX) return MPT_ERROR(BadValue);
		            *((uint32_t *) dest) = *((int64_t *) from); break;
		  case 't': if (*((int64_t *) from) < 0) return MPT_ERROR(BadValue);
		  case 'x': *((int64_t *) dest) = *((int64_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((int64_t *) from); break;
		  case 'D':
		  case 'd': *((double *) dest) = *((int64_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((int64_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 't':
		switch (dtype) {
		  case 'c': if (!isgraph(*((uint64_t *) from))) return MPT_ERROR(BadValue);
		            *((char *) dest) = *((uint64_t *) from); break;
		  case 'b': if (*((uint64_t *) from) > INT8_MAX) return MPT_ERROR(BadValue);
		            *((int8_t *) dest) = *((uint64_t *) from); break;
		  case 'y': if (*((uint64_t *) from) > UINT8_MAX) return MPT_ERROR(BadValue);
		            *((uint8_t *) dest) = *((uint64_t *) from); break;
		  case 'n': if (*((uint32_t *) from) > INT16_MAX) return MPT_ERROR(BadValue);
		            *((int16_t *) dest) = *((uint64_t *) from); break;
		  case 'q': if (*((uint32_t *) from) > UINT16_MAX) return MPT_ERROR(BadValue);
		            *((uint16_t *) dest) = *((uint64_t *) from); break;
		  case 'i': if (*((uint32_t *) from) > INT32_MAX) return MPT_ERROR(BadValue);
		            *((int32_t *) dest) = *((uint64_t *) from); break;
		  case 'u': if (*((uint64_t *) from) > UINT32_MAX) return MPT_ERROR(BadValue);
		            *((uint32_t *) dest) = *((uint64_t *) from); break;
		  case 'x': if (*((uint64_t *) from) > INT64_MAX) return MPT_ERROR(BadValue);
		  case 't': *((int64_t *) dest) = *((uint64_t *) from); break;
		  
		  case 'f': *((float *) dest) = *((uint64_t *) from); break;
		  case 'D':
		  case 'd': *((double *) dest) = *((uint64_t *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((uint64_t *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'f':
		switch (dtype) {
		  case 'f': *((float *) dest) = *((float *) from); break;
		  case 'd': *((double *) dest) = *((float *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((float *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  case 'd':
		switch (dtype) {
		  case 'f': if (*((double *) from) < FLT_MIN || *((double *) from) > FLT_MAX) return MPT_ERROR(BadValue);
		            *((float *) dest) = *((double *) from); break;
		  case 'd': *((double *) dest) = *((double *) from); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
#ifdef _MPT_FLOAT_EXTENDED_H
	  case 'e': {
		long double val = *((long double *) from);
		switch (dtype) {
		  case 'f': if (val < FLT_MIN || val > FLT_MAX) return MPT_ERROR(BadValue);
		            *((float *) dest) = val; break;
		  case 'd': if (val < DBL_MIN || val > DBL_MIN) return MPT_ERROR(BadValue);
		            *((double *) dest) = val; break;
		  case 'e': *((long double *) dest) = val; break;
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  }
#endif
	  default:
		/* vector conversion */
		if (MPT_type_vector(dtype)) {
			struct iovec *vec = dest;
			
			/* copy vector data */
			if (MPT_type_isVector(ftype)) {
				/* require same or generic type */
				if ((dtype & 0x1f) && dtype != ftype) {
					return MPT_ERROR(BadType);
				}
				memcpy(dest, from, flen);
				break;
			}
			/* convert from scalar */
			else if (!MPT_type_isScalar(ftype)
			      || (dtype & 0x1f) != (ftype & 0x1f)) {
				return MPT_ERROR(BadType);
			}
			vec->iov_base = (void *) from;
			vec->iov_len = flen;
			break;
		}
		/* unable to convert */
		else if (dtype != ftype) {
			return MPT_ERROR(BadType);
		}
		/* copy pointer data */
		else if (!dlen) {
			*((void **) dest) = *((void **) from); break;
			dlen = sizeof(void *);
		}
		/* raw data copy */
		else {
			memcpy(dest, from, flen);
		}
	}
	*fptr = from + flen;
	
	return dlen;
}
