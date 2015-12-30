/*!
 * \file
 * mpt type registry
 */

#include <ctype.h>
#include <stdlib.h>

#include <sys/uio.h>
#include <limits.h>

#include "array.h"
#include "convert.h"

#include "../mptplot/layout.h"

static const struct {
	uint8_t key, size;
} static_ptypes[] = {
	/* scalar system types */
	{ MPT_ENUM(TypeProperty), sizeof(MPT_STRUCT(property)) },
	{ MPT_ENUM(TypeValFmt),   sizeof(MPT_STRUCT(valfmt)) },
	
	/* basic layout types */
	{ MPT_ENUM(TypeColor),    sizeof(MPT_STRUCT(color)) },
	{ MPT_ENUM(TypeLineAttr), sizeof(MPT_STRUCT(lineattr)) },
	{ MPT_ENUM(TypeLine),     sizeof(MPT_STRUCT(line)) },
	
	/* basic printable types */
	{ 'c', sizeof(char) },
	
	{ 'b', sizeof(int8_t)  },
	{ 'y', sizeof(uint8_t) },
	
	{ 'n', sizeof(int16_t)  },
	{ 'q', sizeof(uint32_t) },
	
	{ 'i', sizeof(int32_t)  },
	{ 'u', sizeof(uint32_t) },
	
	{ 'x', sizeof(int64_t) },
	{ 't', sizeof(uint64_t) },
	{ 'l', sizeof(uint64_t) },
	
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

static MPT_STRUCT(array) scalars = { 0 };
static int pointers = 0;

static void ptypes_finish(void)
{
	mpt_array_clone(&scalars, 0);
}
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
	/* builtin types */
	if (type < 0) return -3;
	
	if (type < MPT_ENUM(TypeUser)) {
		uint8_t i;
		
		/* generic array */
		if (type == MPT_ENUM(TypeArrBase)) {
			return 0;
		}
		/* typed array */
		if (MPT_value_isArray(type)) {
			if (mpt_valsize(type - 0x20) < 0) {
				return MPT_ERROR(BadType);
			}
			return 0;
		}
		/* basic type */
		for (i = 0; i < MPT_arrsize(static_ptypes); ++i) {
			if (static_ptypes[i].key != (type & 0x7f)) continue;
			if (type & 0x80) return sizeof(struct iovec);
			return static_ptypes[i].size;
		}
		return MPT_ERROR(BadType);
	}
	/* user types */
	if (scalars._buf) {
		ssize_t len = scalars._buf->used / sizeof(size_t);
		type -= MPT_ENUM(TypeUser);
		type -= 0x60; /* scalar type offset */
		if (type < len) {
			return ((size_t *) (scalars._buf+1))[type];
		}
	}
	/* user pointers */
	if (pointers) {
		type -= MPT_ENUM(TypeUser);
		if (type < pointers) {
			return 0;
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
	size_t *sizes, pos;
	
	if (csize > SIZE_MAX/4) return MPT_ERROR(BadValue);
	
	if (!csize) {
		if (pointers >= 0x20) {
			return MPT_ERROR(BadArgument);
		}
		return MPT_ENUM(TypeUser) + pointers++;
	}
	pos = scalars._buf ? scalars._buf->used / sizeof(*sizes) : 0;
	if (pos >= 0x20) {
		return MPT_ERROR(BadArgument);
	};
	if (!(sizes = mpt_array_append(&scalars, sizeof(*sizes), 0))) {
		return MPT_ERROR(BadOperation);
	}
	*sizes = csize;
	
	if (!pos) atexit(ptypes_finish);
	
	return MPT_ENUM(TypeUser) + 0x60 + pos;
}

