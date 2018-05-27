/*!
 * \file
 * mpt type registry
 */

#include "../mptplot/layout.h"
#include "../mptio/connection.h"

#include "convert.h"
#include "object.h"
#include "array.h"
#include "event.h"

#include <stdlib.h>

#include <sys/uio.h>

static const struct {
	uint8_t key, size;
} static_ptypes[] = {
	/* system types (0x1 - 0x7) */
	{ MPT_ENUM(TypeSocket),    sizeof(MPT_STRUCT(socket)) },
	{ MPT_ENUM(TypeFile),      0 },
	{ MPT_ENUM(TypeAddress),   0 },
	
	/* basic types (0x8 - 0xf) */
	{ MPT_ENUM(TypeValFmt),    sizeof(MPT_STRUCT(valfmt)) },
	{ MPT_ENUM(TypeValue),     sizeof(MPT_STRUCT(value)) },
	{ MPT_ENUM(TypeProperty),  sizeof(MPT_STRUCT(property)) },
	{ MPT_ENUM(TypeCommand),   sizeof(MPT_STRUCT(command)) },
	{ MPT_ENUM(TypeNode),      0 },
	{ MPT_ENUM(TypeReplyData), 0 },
	
	/* skip interface data types */
	
	/* layout types (0x10 - 0x1f) */
	{ MPT_ENUM(TypeLineAttr),  sizeof(MPT_STRUCT(lineattr)) },
	{ MPT_ENUM(TypeColor),     sizeof(MPT_STRUCT(color)) },
	{ MPT_ENUM(TypeLine),      sizeof(MPT_STRUCT(line)) },
	{ MPT_ENUM(TypeText),      0 },
	{ MPT_ENUM(TypeAxis),      0 },
	{ MPT_ENUM(TypeWorld),     0 },
	{ MPT_ENUM(TypeGraph),     0 },
	
	/* generic metatype */
	{ MPT_ENUM(TypeMeta),      0 },
	
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
#ifdef _MPT_FLOAT_EXTENDED_H
	{ 'e', sizeof(long double) },
#endif
	/* string types */
	{ 's', 0 },
	{ 'k', 0 },  /* keyword */
	{ 'o', 0 },  /* D-Bus object path */
};

static size_t types[MPT_ENUM(_TypeScalarSize)];
static int types_count = 0;

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
	/* bad type value */
	if (type < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* builtin types */
	if (type < MPT_ENUM(_TypeDynamic)) {
		uint8_t i;
		
		/* generic interface type */
		if (MPT_value_isInterface(type)) {
			return 0;
		}
		/* generic/typed vector */
		if (type == MPT_ENUM(TypeVector)
		    || MPT_value_isVector(type)) {
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
	/* static or dynamic interface */
	if (MPT_value_isInterface(type)) {
		return 0;
	}
	type -= MPT_ENUM(_TypeDynamic);
	/* user generics */
	if (!types_count) {
		return MPT_ERROR(BadValue);
	}
	/* invalid basic type value */
	if (type < MPT_ENUM(_TypeVectorBase)) {
		return MPT_ERROR(BadType);
	}
	/* vector for valid scalar type */
	if (type < MPT_ENUM(_TypeScalarBase)) {
		type -= MPT_ENUM(_TypeVectorBase);
		if (type < types_count) {
			return MPT_ERROR(BadValue);
		}
		return sizeof(struct iovec);
	}
	/* scalar user type range */
	type -= MPT_ENUM(_TypeScalarBase);
	if (type < types_count) {
		return types[type];
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
	if (types_count >= (int) MPT_arrsize(types)) {
		return MPT_ERROR(BadArgument);
	}
	types[types_count] = csize;
	csize = MPT_ENUM(_TypeScalarBase) + types_count++;
	
	return csize + MPT_ENUM(_TypeDynamic);
}

