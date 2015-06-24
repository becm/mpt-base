
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
 * \retval -1  unknown source/target type
 * \retval -2  conversion not in allowd range
 * \retval -3  unknown conversion
 */

extern int mpt_data_convert(const void **fptr, int ftype, void *dest, int dtype)
{
	const uint8_t *from = *fptr;
	int flen, dlen;
	
	/* check type sizes */
	if ((flen = mpt_valsize(ftype)) < 0) return -1;
	if ((dlen = mpt_valsize(dtype)) < 0) return -1;
	
	if (!flen) flen = sizeof(void *);
	if (!dlen) dlen = sizeof(void *);
	
	/* advance source only */
	if (!dest) {
		*fptr = from + flen;
		return dlen;
	}
	/* need conversion */
	if (ftype != dtype) {
		switch (ftype) {
		  case 'c':
		  case 'b':
			switch (dtype) {
			  case 'C': if (!isgraph(((int8_t *) from))) return -2;
			  case 'B': if (*((int8_t *) from) < 0) return -2;
			  case 'c':
			  case 'b': *((int8_t *) dest) = *((int8_t *) from); break;
			  case 'n': case 'N':
			  case 'H': if (*((int8_t *) from) < 0) return -2;
			  case 'h': *((int16_t *) dest) = *((int8_t *) from); break;
			  case 'u': case 'U':
			  case 'I': if (*((int8_t *) from) < 0) return -2;
			  case 'i': *((int32_t *) dest) = *((int8_t *) from); break;
			  case 'x': case 'X': case 't':
			  case 'L': if (*((int8_t *) from) < 0) return -2;
			  case 'l': *((int32_t *) dest) = *((int8_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((int8_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((int8_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((int8_t *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'C':
		  case 'B':
		  case 'y':
		  case 'Y':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((uint8_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((uint8_t *) from); break;
			  case 'C': if (!isgraph(((int8_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': *((uint8_t *) dest) = *((uint8_t *) from); break;
			  case 'H': case 'n': case 'N':
			  case 'h': *((int16_t *) dest) = *((uint8_t *) from); break;
			  case 'I': case 'u': case 'U':
			  case 'i': *((int32_t *) dest) = *((uint8_t *) from); break;
			  case 'L': case 'x': case 'X': case 't':
			  case 'l': *((int64_t *) dest) = *((uint8_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((uint8_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((uint8_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((uint8_t *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'h':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((int16_t *) from) < INT8_MIN || *((int16_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((int8_t *) from); break;
			  case 'C': if (!isgraph(((int8_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((int16_t *) from) < 0 || *((int16_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((int16_t *) from); break;
			  case 'n': case 'N':
			  case 'H': if (*((int16_t *) from) < 0) return -2;
			  case 'h': *((int16_t *) dest) = *((int16_t *) from); break;
			  case 'u': case 'U':
			  case 'I': if (*((int16_t *) from) < 0) return -2;
			  case 'i': *((int32_t *) dest) = *((int16_t *) from); break;
			  case 'x':
			  case 'X': if (*((int16_t *) from) < 0) return -2;
			  case 'l': *((int64_t *) dest) = *((int16_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((int16_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((int16_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'H':
		  case 'n':
		  case 'N':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((uint16_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((uint16_t *) from); break;
			  case 'C': if (!isgraph(((uint16_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((uint16_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((int16_t *) from); break;
			  case 'h': if (*((uint16_t *) from) > INT16_MAX) return -2;
			  case 'n': case 'N':
			  case 'H': *((uint16_t *) dest) = *((uint16_t *) from); break;
			  case 'I': case 'u': case 'U':
			  case 'i': *((int32_t *) dest) = *((int16_t *) from); break;
			  case 'L': case 'x': case 'X': case 't':
			  case 'l': *((int64_t *) dest) = *((int16_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((int16_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((int16_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'i':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((int32_t *) from) < INT8_MIN || *((int32_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((int32_t *) from); break;
			  case 'C': if (!isgraph(((int32_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((int32_t *) from) < 0 || *((int32_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((int32_t *) from); break;
			  case 'h': if (*((int32_t *) from) < INT16_MIN || *((int32_t *) from) > INT16_MAX) return -2;
				    *((int16_t *) dest) = *((int32_t *) from); break;
			  case 'n': case 'N':
			  case 'H': if (*((int32_t *) from) < 0 || *((int32_t *) from) > UINT16_MAX) return -2;
				    *((uint16_t *) dest) = *((int32_t *) from); break;
			  case 'u': case 'U':
			  case 'I': if (*((int32_t *) from) < 0) return -2;
			  case 'i': *((int32_t *) dest) = *((int32_t *) from); break;
			  case 'x': case 'X': case 't':
			  case 'L': if (*((int32_t *) from) < 0) return -2;
			  case 'l': *((int64_t *) dest) = *((int32_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((int32_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((int32_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((int32_t *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'I':
		  case 'u':
		  case 'U':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((uint32_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((uint32_t *) from); break;
			  case 'C': if (!isgraph(((uint32_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((uint32_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((uint32_t *) from); break;
			  case 'h': if (*((uint32_t *) from) > INT16_MAX) return -2;
			  case 'n': case 'N':
			  case 'H': *((uint16_t *) dest) = *((uint32_t *) from); break;
			  case 'i': if (*((uint32_t *) from) > INT32_MAX) return -2;
			  case 'u':  case 'U':
			  case 'I': *((uint32_t *) dest) = *((uint32_t *) from); break;
			  case 'L': case 'x': case 'X': case 't':
			  case 'l': *((int64_t *) dest) = *((uint32_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((uint32_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((uint32_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((uint32_t *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'l':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((int64_t *) from) < INT8_MIN || *((int64_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((int64_t *) from); break;
			  case 'C': if (!isgraph(((int64_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((int64_t *) from); break;
			  case 'h': if (*((int64_t *) from) < INT16_MIN || *((int64_t *) from) > INT16_MAX) return -2;
				    *((int16_t *) dest) = *((int64_t *) from); break;
			  case 'n': case 'N':
			  case 'H': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT16_MAX) return -2;
				    *((uint16_t *) dest) = *((int64_t *) from); break;
			  case 'u': case 'U':
			  case 'I': if (*((int64_t *) from) < 0 || *((int64_t *) from) > UINT32_MAX) return -2;
				    *((uint32_t *) dest) = *((int64_t *) from); break;
			  case 'i': if (*((int64_t *) from) < INT32_MIN || *((int64_t *) from) > INT32_MAX) return -2;
				    *((int32_t *) dest) = *((int64_t *) from); break;
			  case 'x': case 'X': case 't':
			  case 'L': if (*((int64_t *) from) < 0) return -2;
			  case 'l': *((int64_t *) dest) = *((int64_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((int64_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((int64_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'D': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'L':
		  case 'x':
		  case 'X':
		  case 't':
			switch (dtype) {
			  case 'c':
			  case 'b': if (*((uint64_t *) from) > INT8_MAX) return -2;
				    *((int8_t *) dest) = *((uint64_t *) from); break;
			  case 'C': if (!isgraph(((uint64_t *) from))) return -2;
			  case 'y': case 'Y':
			  case 'B': if (*((uint64_t *) from) > UINT8_MAX) return -2;
				    *((uint8_t *) dest) = *((uint64_t *) from); break;
			  case 'h': if (*((uint32_t *) from) > INT16_MAX) return -2;
			  case 'H': *((uint16_t *) dest) = *((uint64_t *) from); break;
			  case 'i': if (*((uint32_t *) from) > INT32_MAX) return -2;
			  case 'u': case 'U':
			  case 'I': if (*((uint64_t *) from) > UINT32_MAX) return -2;
				    *((uint32_t *) dest) = *((uint64_t *) from); break;
			  case 'l': if (*((uint64_t *) from) > INT64_MAX) return -2;
			  case 'x': case 'X': case 't':
			  case 'L': *((int64_t *) dest) = *((uint64_t *) from); break;
			  case 'F':
			  case 'f': *((float *) dest) = *((uint64_t *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((uint64_t *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'f':
		  case 'F':
			switch (dtype) {
			  case 'F':
			  case 'f': *((float *) dest) = *((float *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((float *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
		  case 'd':
		  case 'D':
			switch (dtype) {
			  case 'F':
			  case 'f': if (*((double *) from) < FLT_MIN || *((double *) from) > FLT_MAX) return -2;
				    *((float *) dest) = *((double *) from); break;
			  case 'D':
			  case 'd': *((double *) dest) = *((double *) from); break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
			  case 'E':
			  case 'e': *((long double *) dest) = *((double *) from); break;
#endif
			  default: return -3; /* invalid conversion */
			}
			break;
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
		  case 'e':
		  case 'E':
			switch (dtype) {
			  case 'F':
			  case 'f': if (*((double *) from) < FLT_MIN || *((double *) from) > FLT_MAX) return -2;
				    *((float *) dest) = *((double *) from); break;
			  case 'D':
			  case 'd': if (*((long double *) from) < DBL_MIN || *((long double *) from) > DBL_MIN) return -2;
				    *((double *) dest) = *((long double *) from); break;
			  case 'E':
			  case 'e': *((long double *) dest) = *((long double *) from); break;
			  default: return -3; /* invalid conversion */
			}
			break;
#endif
		  /* allow metatype downgrade */
		  case MPT_ENUM(TypeOutput):
		  case MPT_ENUM(TypeCycle):
		  case MPT_ENUM(TypeSolver):
			if (dtype != MPT_ENUM(TypeMeta)) return -3;
			*((void **) dest) = *((void **) from);
			break;
		  default: return -3; /* invalid source type */
		}
	}
	else {
		memcpy(dest, from, flen);
	}
	*fptr = from + flen;
	
	return dlen;
}
