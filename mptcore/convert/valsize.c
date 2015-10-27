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
	/* core types */
	{ MPT_ENUM(TypeSocket),  0 },
	{ MPT_ENUM(TypeAddress), 0 },
	
	/* data copy types */
	{ MPT_ENUM(TypeValue),    sizeof(MPT_STRUCT(value)) },
	{ MPT_ENUM(TypeProperty), sizeof(MPT_STRUCT(property)) },
	
	/* data pointer types */
	{ MPT_ENUM(TypeNode),  0 },
	{ MPT_ENUM(TypeArray), 0 },
	
	/* basic layout types */
	{ MPT_ENUM(TypeColor),    sizeof(MPT_STRUCT(color)) },
	{ MPT_ENUM(TypeLineAttr), sizeof(MPT_STRUCT(lineattr)) },
	{ MPT_ENUM(TypeLine),     sizeof(MPT_STRUCT(line)) },
	/* extended layout types */
	{ MPT_ENUM(TypeAxis),   0 },
	{ MPT_ENUM(TypeGraph),  0 },
	{ MPT_ENUM(TypeWorld),  0 },
	{ MPT_ENUM(TypeText),   0 },
	
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
	
	/* reference types */
	{ MPT_ENUM(TypeIODevice),  0 },
	{ MPT_ENUM(TypeInput),     0 },
	{ MPT_ENUM(TypeGroup),     0 },
	{ MPT_ENUM(TypeCycle),     0 },
	
	/* metatype references */
	{ MPT_ENUM(TypeMeta),   0 },
	{ MPT_ENUM(TypeOutput), 0 },
	{ MPT_ENUM(TypeSolver), 0 }
};

static MPT_STRUCT(array) ptypes = { 0 };

static void ptypes_finish(void)
{
	mpt_array_clone(&ptypes, 0);
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
		if (type == MPT_ENUM(TypeArray)) {
			return 0;
		}
		/* typed array */
		if (isupper(type)) {
			if (mpt_valsize(tolower(type)) < 0) {
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
	if (ptypes._buf) {
		ssize_t len = ptypes._buf->used / sizeof(size_t);
		type -= MPT_ENUM(TypeUser);
		if (type < len) {
			return ((size_t *) (ptypes._buf+1))[type];
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
	
	if (csize > SIZE_MAX/4) return -2;
	
	pos = ptypes._buf ? ptypes._buf->used / sizeof(*sizes) : 0;
	
	if (pos > (MPT_ENUM(_TypeFinal) - MPT_ENUM(TypeUser))) return -2;
	if (!(sizes = mpt_array_append(&ptypes, sizeof(*sizes), 0))) return -1;
	*sizes = csize;
	
	if (!pos) atexit(ptypes_finish);
	
	return MPT_ENUM(TypeUser) + pos;
}

