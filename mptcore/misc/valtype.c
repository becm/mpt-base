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
static MPT_STRUCT(array) metatype_names = MPT_ARRAY_INIT;

static int interface_limit = MPT_ENUM(_TypeInterfaceMax);
static int metatype_limit  = MPT_ENUM(_TypeMetaMax);

static void clearArrays(void)
{
	mpt_array_clone(&interface_names, 0);
	mpt_array_clone(&metatype_names, 0);
}

static const struct {
	int type;
	const char name[28];
} static_types[] = {
	/* interface types */
	{ MPT_ENUM(TypeLogger),  "logger"   },
	{ MPT_ENUM(TypeObject),  "object"   },
	{ MPT_ENUM(TypeOutput),  "output"   },
	{ MPT_ENUM(TypeConfig),  "config"   }
};

static int addArrayIdentifier(MPT_STRUCT(array) *arr, const char *name)
{
	MPT_STRUCT(identifier) *id;
	static int reg = 0;
	
	if (!reg) {
		atexit(clearArrays);
		++reg;
	}
	if (!(id = mpt_array_append(arr, sizeof(*id), 0))) {
		return MPT_ERROR(BadOperation);
	}
	id->_max = sizeof(id->_val) + sizeof(id->_base);
	if (!mpt_identifier_set(id, name, -1)) {
		arr->_buf->_used -= sizeof(*id);
		return MPT_ERROR(BadValue);
	}
	return 0;
}
static const char *getArrayName(MPT_STRUCT(array) *arr, int pos)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(identifier) *id;
	int count;
	
	count = 0;
	if ((buf = arr->_buf)) {
		count = buf->_used / sizeof(*id);
	}
	if (count <= pos) {
		return 0;
	}
	id = (void *) (buf + 1);
	return mpt_identifier_data(id + pos);
}


/*!
 * \ingroup mptConvert
 * \brief get type name
 * 
 * Get name for previously registered type name.
 * 
 * \return name for type ID
 */
extern const char *mpt_valtype_name(int type)
{
	int i, max;
	
	if (type > interface_limit
	    && type <= MPT_ENUM(_TypeInterfaceMax)) {
		return "";
	}
	if (type > metatype_limit
	    && type <= MPT_ENUM(_TypeMetaMax)) {
		return "";
	}
	for (i = 0, max = MPT_arrsize(static_types); i < max; ++i) {
		if (type == static_types[i].type) {
			return static_types[i].name;
		}
	}
	if (type >= MPT_ENUM(_TypeMetaBase)
	    && type <= MPT_ENUM(_TypeMetaMax)) {
		return getArrayName(&metatype_names, max - MPT_ENUM(_TypeMetaBase));
	}
	if (type >= MPT_ENUM(_TypeInterfaceBase)
	    && type <= MPT_ENUM(_TypeInterfaceMax)) {
		return getArrayName(&interface_names, type - MPT_ENUM(_TypeInterfaceBase));
	}
	errno = EINVAL;
	return 0;
}

/*!
 * \ingroup mptConvert
 * \brief get type name
 * 
 * Get name for previously registered type name.
 * 
 * \return name for type ID
 */
extern int mpt_valtype_id(const char *name, int len)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(identifier) *id;
	size_t i, max;
	
	if (!name || !len || !*name) {
		return MPT_ERROR(BadArgument);
	}
	/* exact length match for type name */
	if (len >= 0) {
		for (i = 0, max = MPT_arrsize(static_types); i < max; ++i) {
			if (len == (int) strlen(static_types[i].name)
			    && !strncmp(name, static_types[i].name, len)) {
				return static_types[i].type;
			}
		}
		if ((buf = interface_names._buf)) {
			id = (void *) (buf + 1);
			for (i = 0, max = buf->_used / sizeof(*id); i < max; ++i) {
				const char *curr;
				if ((curr = mpt_identifier_data(id++))
				    && len == (int) strlen(curr)
				    && !strncmp(name, curr, len)) {
					return i + MPT_ENUM(_TypeBaseDynamic);
				}
			}
		}
		if ((buf = metatype_names._buf)) {
			id = (void *) (buf + 1);
			for (i = 0, max = buf->_used / sizeof(*id); i < max; ++i) {
				const char *curr;
				if ((curr = mpt_identifier_data(id++))
				    && len == (int) strlen(curr)
				    && !strncmp(name, curr, len)) {
					return i + MPT_ENUM(_TypeMetaBase);
				}
			}
		}
		return MPT_ERROR(BadType);
	}
	/* shortnames */
	if (!strcmp(name, "log")) {
		return MPT_ENUM(TypeLogger);
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
	if ((buf = interface_names._buf)) {
		id = (void *) (buf + 1);
		for (i = 0, max = buf->_used / sizeof(*id); i < max; ++i) {
			const char *curr;
			if ((curr = mpt_identifier_data(id++))
			    && !strcmp(name, curr)) {
				return i + MPT_ENUM(_TypeBaseDynamic);
			}
		}
	}
	if ((buf = metatype_names._buf)) {
		id = (void *) (buf + 1);
		for (i = 0, max = buf->_used / sizeof(*id); i < max; ++i) {
			const char *curr;
			if ((curr = mpt_identifier_data(id++))
			    && !strcmp(name, curr)) {
				return i + MPT_ENUM(_TypeMetaBase);
			}
		}
	}
	return MPT_ERROR(BadType);
	
}

/*!
 * \ingroup mptConvert
 * \brief register metatype
 * 
 * Register new global metatype.
 * 
 * \return type code of new object type
 */
extern int mpt_valtype_meta_new(const char *name)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(identifier) *id;
	int count, err;
	
	count = 0;
	if ((buf = metatype_names._buf)) {
		count = buf->_used / sizeof(*id);
	}
	if (!name) {
		if (metatype_limit > count) {
			return metatype_limit--;
		}
		return MPT_ERROR(BadOperation);
	}
	if (strlen(name) < 4 || mpt_valtype_id(name, -1) >= 0) {
		return MPT_ERROR(BadValue);
	}
	if (count >= MPT_ENUM(_TypeDynamicMax)) {
		return MPT_ERROR(MissingBuffer);
	}
	if ((err = addArrayIdentifier(&metatype_names, name)) < 0) {
		return err;
	}
	return MPT_ENUM(_TypeMetaBase) + count;
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
	int count, err;
	
	count = 0;
	if ((buf = interface_names._buf)) {
		count = buf->_used / sizeof(MPT_STRUCT(identifier));
	}
	if (!name) {
		if (interface_limit > count) {
			return interface_limit--;
		}
		return MPT_ERROR(BadOperation);
	}
	if (!name[0] || !name[1]) {
		return MPT_ERROR(BadValue);
	}
	if (mpt_valtype_id(name, -1) >= 0) {
		return MPT_ERROR(BadEncoding);
	}
	count += MPT_ENUM(_TypeInterfaceBase);
	if (count >= interface_limit) {
		return MPT_ERROR(MissingBuffer);
	}
	if ((err = addArrayIdentifier(&interface_names, name)) < 0) {
		return err;
	}
	return count;
}
