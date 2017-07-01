/*!
 * \file
 * mpt type registry
 */

#include "../mptplot/layout.h"

#include "convert.h"
#include "array.h"

#include <stdlib.h>

#include <sys/uio.h>

# define MPT_TYPE_LIMIT (0x80 / 4)

static const struct {
	uint8_t key, size;
} static_ptypes[] = {
	/* system types (0x1 - 0x7) */
	{ MPT_ENUM(TypeSocket),   sizeof(MPT_STRUCT(socket)) },
	{ MPT_ENUM(TypeFile),     0 },
	{ MPT_ENUM(TypeAddress),  0 },
	
	/* basic types (0x8 - 0xf) */
	{ MPT_ENUM(TypeValFmt),   sizeof(MPT_STRUCT(valfmt)) },
	{ MPT_ENUM(TypeValue),    sizeof(MPT_STRUCT(value)) },
	{ MPT_ENUM(TypeProperty), sizeof(MPT_STRUCT(property)) },
	{ MPT_ENUM(TypeNode),     0 },
	{ MPT_ENUM(TypeArray),    0 },
	{ MPT_ENUM(TypeCommand),  0 },
	
	/* skip reference and object data types */
	
	/* layout types (0x30 - 0x39) */
	{ MPT_ENUM(TypeLineAttr), sizeof(MPT_STRUCT(lineattr)) },
	{ MPT_ENUM(TypeColor),    sizeof(MPT_STRUCT(color)) },
	{ MPT_ENUM(TypeLine),     sizeof(MPT_STRUCT(line)) },
	{ MPT_ENUM(TypeText),     0 },
	{ MPT_ENUM(TypeAxis),     0 },
	{ MPT_ENUM(TypeWorld),    0 },
	{ MPT_ENUM(TypeGraph),    0 },
	
	/* basic printable types */
	{ 'c', sizeof(char) },
	
	{ 'b', sizeof(int8_t)  },
	{ 'y', sizeof(uint8_t) },
	
	{ 'n', sizeof(int16_t)  },
	{ 'q', sizeof(uint16_t) },
	
	{ 'i', sizeof(int32_t)  },
	{ 'u', sizeof(uint32_t) },
	
	{ 'x', sizeof(int64_t) },
	{ 't', sizeof(uint64_t) },
	
	{ 'l', sizeof(long) },
	
	{ 'f', sizeof(float) },
	{ 'd', sizeof(double) },
	{ MPT_ENUM(TypeFloat80), sizeof(MPT_STRUCT(float80)) },
#ifdef _MPT_FLOAT_EXTENDED_H
	{ 'e', sizeof(long double) },
#endif
	/* string types */
	{ 's', 0 },
	{ 'k', 0 },  /* keyword */
	{ 'o', 0 },  /* D-Bus object path */
};

static size_t generics[MPT_TYPE_LIMIT];
static int generics_count = 0;

/*!
 * \ingroup mptConvert
 * \brief get size of registered type
 * 
 * Get size of builtin or user type registered by mpt_valtype_add()
 * or dynamic object/reference type.
 * 
 * \param type  type identifier
 * 
 * \return size of registered type (0 if pointer or reference type)
 */
extern ssize_t mpt_valsize(int type)
{
	int base;
	
	/* bad type value */
	if (type < 0
	    || type > MPT_ENUM(_TypeFinal)) {
		return MPT_ERROR(BadArgument);
	}
	/* builtin types */
	if (type < MPT_ENUM(_TypeDynamic)) {
		uint8_t i;
		
		/* generic interface type */
		if (MPT_value_isUnrefable(type)) {
			return 0;
		}
		/* generic/typed vector */
		if (MPT_value_isVector(type)) {
			return sizeof(struct iovec);
		}
		/* basic type */
		for (i = 0; i < MPT_arrsize(static_ptypes); ++i) {
			if (static_ptypes[i].key == type) {
				return static_ptypes[i].size;
			}
		}
		return MPT_ERROR(BadType);
	}
	type -= MPT_ENUM(_TypeDynamic);
	
	/* type is registered reference (or object) */
	if ((base = mpt_valtype_isref(type)) >= 0) {
		return 0;
	}
	/* user generics */
	if (generics_count) {
		int base;
		
		if ((type - MPT_ENUM(TypeScalBase)) >= generics_count) {
			return MPT_ERROR(BadType);
		}
		/* vector for registered user type */
		if ((base = MPT_value_fromVector(type)) >= 0) {
			type -= MPT_ENUM(TypeVector);
			return (type < generics_count) ? (int) sizeof(struct iovec) : MPT_ERROR(BadValue);
		}
		/* scalar type offset */
		base = type - MPT_ENUM(TypeScalBase);
		if (base < generics_count) {
			return generics[base];
		}
	}
	return MPT_ERROR(BadValue);
}

/*!
 * \ingroup mptConvert
 * \brief register new type
 * 
 * register new user type to use with mpt_valsize()
 * 
 * \param csize  size of new type (0 for pointer type)
 * 
 * \return type code of new user type
 */
extern int mpt_valtype_add(size_t csize)
{
	if (csize > SIZE_MAX/4) {
		return MPT_ERROR(BadValue);
	}
	if (generics_count >= (int) MPT_arrsize(generics)) {
		return MPT_ERROR(BadArgument);
	}
	generics[generics_count] = csize;
	
	return MPT_ENUM(_TypeDynamic) + MPT_ENUM(TypeScalBase) + generics_count++;
}

