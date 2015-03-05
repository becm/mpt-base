/*!
 * \file
 * mpt type registry
 */

#include <stdlib.h>

#include <sys/uio.h>
#include <limits.h>

#include "array.h"
#include "plot.h"
#include "convert.h"

static const struct {
	uint8_t key, size;
} static_ptypes[] = {
	/* core types */
	{ MPT_ENUM(TypeSocket),  0 },
	{ MPT_ENUM(TypeAddress), 0 },
	
	/* data copy types */
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
	
	/* basic types */
	{ 'c', sizeof(char) },
	{ 'C', sizeof(unsigned char) },
	{ 'b', sizeof(int8_t) },
	{ 'B', sizeof(uint8_t) },
	{ 'h', sizeof(int16_t) },
	{ 'H', sizeof(uint16_t) },
	{ 'i', sizeof(int32_t) },
	{ 'I', sizeof(uint32_t) }, { 'o', sizeof(uint32_t) }, { 'x', sizeof(uint32_t) },
	{ 'l', sizeof(int64_t) },
	{ 'L', sizeof(uint64_t) }, { 'O', sizeof(uint64_t) }, { 'X', sizeof(uint64_t) },
	{ 'f', sizeof(float) },    { 'g', sizeof(float) },
	{ 'd', sizeof(double) },   { 'G', sizeof(double) },   { 'F', sizeof(double) },
#if _XOPEN_SOURCE >= 600 || defined(_ISOC99_SOURCE)
	{ 'D', sizeof(long double) },
#endif
	/* different string representations */
	{ 's', 0 },
	{ 'k', 0 },
	
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
		for (i = 0; i < MPT_arrsize(static_ptypes); ++i) {
			if (static_ptypes[i].key != (type & 0x7f)) continue;
			if (type & 0x80) return sizeof(struct iovec);
			return static_ptypes[i].size;
		}
		return -1;
	}
	/* user types */
	if (ptypes._buf) {
		ssize_t len = ptypes._buf->used / sizeof(size_t);
		type -= MPT_ENUM(TypeUser);
		if (type < len) {
			return ((size_t *) (ptypes._buf+1))[type];
		}
	}
	return -2;
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

