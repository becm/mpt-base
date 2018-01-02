/*!
 * \file
 * mpt type registry
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "meta.h"
#include "array.h"

#include "convert.h"

static MPT_STRUCT(array) interface_names = MPT_ARRAY_INIT;
static MPT_STRUCT(array) metatype_names  = MPT_ARRAY_INIT;

static int interface_limit = MPT_ENUM(_TypeInterfaceMax);
static int metatype_limit  = MPT_ENUM(_TypeMetaMax);

static void clearArrays(void)
{
	mpt_array_clone(&interface_names, 0);
	mpt_array_clone(&metatype_names, 0);
}
struct Element
{
	MPT_STRUCT(identifier) id;
	uint8_t data[48 - sizeof(MPT_STRUCT(identifier))];
};

static const struct {
	int type;
	const char name[28];
} static_types[] = {
	/* modify interface types */
	{ MPT_ENUM(TypeObject),   "object"   },
	{ MPT_ENUM(TypeConfig),   "config"   },
	/* input interface types */
	{ MPT_ENUM(TypeIterator), "iterator" },
	{ MPT_ENUM(TypeSolver),   "solver",  },
	/* output interfaces */
	{ MPT_ENUM(TypeLogger),   "logger"   },
	{ MPT_ENUM(TypeReply),    "reply"    },
	{ MPT_ENUM(TypeOutput),   "output"   },
};

static int addArrayIdentifier(MPT_STRUCT(array) *arr, const char *name)
{
	struct Element *elem;
	static size_t size = sizeof(*elem) - MPT_IDENTIFIER_HSIZE;
	static int reg = 0;
	size_t len;
	
	if (!reg) {
		atexit(clearArrays);
		++reg;
	}
	if (!name) {
		len = 0;
	}
	else if ((len = strlen(name)) >= size) {
		return MPT_ERROR(BadValue);
	}
	if (!(elem = mpt_array_append(arr, sizeof(*elem), 0))) {
		return MPT_ERROR(BadOperation);
	}
	elem->id._max = size;
	if (name) {
		memcpy(elem->id._val, name, len);
		elem->id._len = len + 1;
	}
	return (arr->_buf->_used / sizeof(*elem)) - 1;
}
static const char *getArrayName(const MPT_STRUCT(array) *arr, int pos)
{
	const MPT_STRUCT(buffer) *buf;
	struct Element *elem;
	int count;
	
	count = 0;
	if ((buf = arr->_buf)) {
		count = buf->_used / sizeof(*elem);
	}
	if (count <= pos) {
		return 0;
	}
	elem = (void *) (buf + 1);
	return elem->id._val;
}
static int getArrayId(const MPT_STRUCT(array) *arr, const char *name, int len)
{
	const MPT_STRUCT(buffer) *buf;
	struct Element *elem;
	size_t i, max;
	
	if (!name) {
		return MPT_ERROR(MissingData);
	}
	if (!(buf = arr->_buf)) {
		return MPT_ERROR(MissingBuffer);
	}
	elem = (void *) (buf + 1);
	max = buf->_used / sizeof(*elem);
	if (len < 0) {
		len = strlen(name);
	}
	for (i = 0; i < max; ++i) {
		if (elem[i].id._len != len + 1) {
			continue;
		}
		if (!memcmp(name, elem[i].id._val, len)) {
			return i;
		}
	}
	return MPT_ERROR(BadValue);
}


/*!
 * \ingroup mptConvert
 * \brief get metatype name
 * 
 * Get name for previously registered metatype.
 * 
 * \return name for metatype ID
 */
extern const char *mpt_meta_typename(int type)
{
	const char *name;
	
	if (type > MPT_ENUM(_TypeMetaMax)
	    || type < MPT_ENUM(_TypeMetaBase)) {
		errno = EINVAL;
		return 0;
	}
	if (type > metatype_limit) {
		return "";
	}
	if (!(name = getArrayName(&metatype_names, type - MPT_ENUM(_TypeMetaBase)))) {
		errno = EAGAIN;
	}
	return name;
}
/*!
 * \ingroup mptConvert
 * \brief get interface name
 * 
 * Get name for builtin or previously registered interface.
 * 
 * \return name for interface ID
 */
extern const char *mpt_interface_typename(int type)
{
	const char *name;
	int i, max;
	
	if (type > MPT_ENUM(_TypeInterfaceMax)
	    || type < MPT_ENUM(_TypeInterfaceBase)) {
		errno = EINVAL;
		return 0;
	}
	if (type > interface_limit) {
		return "";
	}
	for (i = 0, max = MPT_arrsize(static_types); i < max; ++i) {
		if (type == static_types[i].type) {
			return static_types[i].name;
		}
	}
	if (!(name = getArrayName(&interface_names, type - MPT_ENUM(_TypeInterfaceBase)))) {
		errno = EAGAIN;
	}
	return name;
}

/*!
 * \ingroup mptConvert
 * \brief get type name
 * 
 * Get name for previously registered type name.
 * Metatype entries take precedence over interfaces.
 * 
 * \return type ID for registered name
 */
extern int mpt_valtype_id(const char *name, int len)
{
	size_t i, max;
	int pos;
	
	if (!name || !len || !*name) {
		return MPT_ERROR(BadArgument);
	}
	/* prefere metatype registrations */
	if ((pos = getArrayId(&metatype_names, name, len)) >= 0) {
		return pos + MPT_ENUM(_TypeMetaBase);
	}
	/* exact length match for interface name */
	if (len >= 0) {
		for (i = 0, max = MPT_arrsize(static_types); i < max; ++i) {
			if (len == (int) strlen(static_types[i].name)
			    && !strncmp(name, static_types[i].name, len)) {
				return static_types[i].type;
			}
		}
		if ((pos = getArrayId(&interface_names, name, len)) >= 0) {
			return pos + MPT_ENUM(_TypeInterfaceBase);
		}
		return MPT_ERROR(BadValue);
	}
	/* shortnames */
	if (!strcmp(name, "log")) {
		return MPT_ENUM(TypeLogger);
	}
	if (!strcmp(name, "iter")) {
		return MPT_ENUM(TypeIterator);
	}
	if (!strcmp(name, "out")) {
		return MPT_ENUM(TypeOutput);
	}
	if (!strcmp(name, "meta") || !strcmp(name, "metatype")) {
		return MPT_ENUM(TypeMeta);
	}
	/* full names without length limit */
	for (i = 0, max = MPT_arrsize(static_types); i < max; ++i) {
		if (!strcmp(name, static_types[i].name)) {
			return static_types[i].type;
		}
	}
	if ((pos = getArrayId(&interface_names, name, len)) >= 0) {
		return pos + MPT_ENUM(_TypeInterfaceBase);
	}
	return MPT_ERROR(BadValue);
	
}

/*!
 * \ingroup mptConvert
 * \brief register metatype
 * 
 * Register name for new global metatype.
 * 
 * \return unique type code
 */
extern int mpt_valtype_meta_new(const char *name)
{
	MPT_STRUCT(buffer) *buf;
	int id, err;
	
	id = MPT_ENUM(_TypeMetaBase);
	if ((buf = metatype_names._buf)) {
		id += buf->_used / sizeof(struct Element);
	}
	if (id > metatype_limit) {
		return MPT_ERROR(MissingBuffer);
	}
	if (!name) {
		return metatype_limit--;
	}
	if (strlen(name) < 4) {
		return MPT_ERROR(BadValue);
	}
	if (getArrayId(&metatype_names, name, -1) >= 0) {
		return MPT_ERROR(BadType);
	}
	if ((err = addArrayIdentifier(&metatype_names, name)) < 0) {
		return err;
	}
	return id;
}
/*!
 * \ingroup mptConvert
 * \brief register object
 * 
 * Register new interface type to use with mpt_valsize()
 * and other internal type operations.
 * 
 * \return type code of new object type
 */
extern int mpt_valtype_interface_new(const char *name)
{
	MPT_STRUCT(buffer) *buf;
	int id, err;
	
	id = MPT_ENUM(_TypeInterfaceBase);
	if ((buf = interface_names._buf)) {
		id += buf->_used / sizeof(struct Element);
	}
	if (id >= interface_limit) {
		return MPT_ERROR(MissingBuffer);
	}
	if (!name) {
		return interface_limit--;
	}
	if (!name[0] || !name[1]) {
		return MPT_ERROR(BadValue);
	}
	if (getArrayId(&interface_names, name, -1) >= 0) {
		return MPT_ERROR(BadType);
	}
	if ((err = addArrayIdentifier(&interface_names, name)) < 0) {
		return err;
	}
	return id;
}
