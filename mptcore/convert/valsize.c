/*!
 * \file
 * mpt type registry
 */

#include "convert.h"
#include "array.h"

#include <stdlib.h>

#include <sys/uio.h>

#include "../mptplot/layout.h"

static const struct {
	uint8_t key, size;
} static_ptypes[] = {
	/* scalar system types */
	{ MPT_ENUM(TypeSocket),   sizeof(MPT_STRUCT(socket)) },
	{ MPT_ENUM(TypeValue),    sizeof(MPT_STRUCT(value)) },
	{ MPT_ENUM(TypeProperty), sizeof(MPT_STRUCT(property)) },
	
	/* basic layout types */
	{ MPT_ENUM(TypeColor),    sizeof(MPT_STRUCT(color)) },
	{ MPT_ENUM(TypeLineAttr), sizeof(MPT_STRUCT(lineattr)) },
	{ MPT_ENUM(TypeLine),     sizeof(MPT_STRUCT(line)) },
	
	/* number format */
	{ MPT_ENUM(TypeValFmt),   sizeof(MPT_STRUCT(valfmt)) },
	
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

static size_t scalars_size[0x80/4];
static int scalars = 0;
static int pointers = 0;

/*!
 * \ingroup mptConvert
 * \brief get size of registered type
 * 
 * get size of builtin or user type registered by mpt_valtype_add()
 * 
 * \param type  type identifier
 * 
 * \return size of registered type (0 if pointer type)
 */
extern ssize_t mpt_valsize(int type)
{
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
		/* generic/typed array */
		if (MPT_value_isArray(type)) {
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
	/* user pointers */
	if (pointers) {
		if (type < pointers) {
			return 0;
		}
	}
	/* user scalars */
	if (scalars) {
		int base;
		
		if ((type - MPT_ENUM(TypeScalBase)) < scalars) {
			return MPT_ERROR(BadType);
		}
		if ((base = MPT_value_fromArray(type)) >= 0) {
			type -= MPT_ENUM(TypeArrBase);
			return (type < scalars) ? 0 : MPT_ERROR(BadValue);
		}
		if ((base = MPT_value_fromVector(type)) >= 0) {
			type -= MPT_ENUM(TypeVecBase);
			return (type < scalars) ? (int) sizeof(struct iovec) : MPT_ERROR(BadValue);
		}
		/* scalar type offset */
		base = type - MPT_ENUM(TypeScalBase);
		if (base < scalars) {
			return scalars_size[base];
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
	if (csize > SIZE_MAX/4) return MPT_ERROR(BadValue);
	
	if (!csize) {
		if (pointers >= MPT_ENUM(TypeVecBase)) {
			return MPT_ERROR(BadArgument);
		}
		return MPT_ENUM(_TypeDynamic) + pointers++;
	}
	if (scalars >= (int) MPT_arrsize(scalars_size)) {
		return MPT_ERROR(BadArgument);
	}
	scalars_size[scalars] = csize;
	
	return MPT_ENUM(_TypeDynamic) + MPT_ENUM(TypeScalBase) + scalars++;
}

