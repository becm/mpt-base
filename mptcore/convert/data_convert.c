
#include <ctype.h>
#include <string.h>
#include <float.h>

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
 */

extern int mpt_data_convert(const void **fptr, int ftype, void *dest, int dtype)
{
	const uint8_t *from = *fptr;
	int flen, dlen;
	
	/* check type sizes */
	if ((flen = mpt_valsize(ftype)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if ((ftype != dtype)
	    && (dlen = mpt_valsize(dtype)) < 0) {
		return MPT_ERROR(BadArgument);
	} else {
		dlen = flen;
	}
	if (!flen) flen = sizeof(void *);
	
	/* advance source only */
	if (!dest) {
		*fptr = from + flen;
		return dlen ? dlen : (int) sizeof(void *);
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((int8_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((uint8_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((int16_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((uint16_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((int32_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((uint32_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((int64_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((uint64_t *) from);
			  mpt_float80_encode(1, &val, dest); break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			  long double val = *((float *) from);
			  mpt_float80_encode(1, &val, dest);
			  break;
		  }
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
		  case MPT_ENUM(TypeFloat80): {
			    long double val = *((double *) from);
			    mpt_float80_encode(1, &val, dest);
			    break;
		  }
		  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
#ifdef _MPT_FLOAT_EXTENDED_H
	  case MPT_ENUM(TypeFloat80):
		if (dtype == MPT_ENUM(TypeFloat80)) {
			memcpy(dest, from, sizeof(MPT_STRUCT(float80)));
		} else {
			long double val;
			mpt_float80_decode(1, (const void *) from, &val);
			
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
	  case 'e': {
		long double val = *((long double *) from);
		switch (dtype) {
		  case 'f': if (val < FLT_MIN || val > FLT_MAX) return MPT_ERROR(BadValue);
		            *((float *) dest) = val; break;
		  case 'd': if (val < DBL_MIN || val > DBL_MIN) return MPT_ERROR(BadValue);
		            *((double *) dest) = val; break;
		  case MPT_ENUM(TypeFloat80):
		            mpt_float80_encode(1, &val, dest); break;
		  case 'e': *((long double *) dest) = val; break;
		  default: return MPT_ERROR(BadType); /* invalid conversion */
		}
		break;
	  }
#endif
	  /* allow metatype downgrade */
	  case MPT_ENUM(TypeOutput):
	  case MPT_ENUM(TypeCycle):
	  case MPT_ENUM(TypeSolver):
		if (dtype == MPT_ENUM(TypeMeta)) {
			ftype = MPT_ENUM(TypeMeta);
		}
	  default:
		/* unable to convert */
		if (dtype != ftype) {
			return MPT_ERROR(BadType);
		}
		if (!dlen) {
			*((void **) dest) = *((void **) from); break;
			dlen = sizeof(void *);
		} else {
			memcpy(dest, from, flen);
		}
	}
	*fptr = from + flen;
	return dlen;
}
