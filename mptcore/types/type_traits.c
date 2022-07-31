/*!
 * \file
 * mpt type registry
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "convert.h"
#include "meta.h"
#include "object.h"
#include "array.h"
#include "event.h"

#include "types.h"

#define basic_type(t, s) { sizeof(s), (t) }
#define pointer_type(t)  { sizeof(void *), (t) }

static const struct {
	const uint8_t size, type;
}
core_sizes[] = {
	/* system types (0x1 - 0x3) */
	basic_type(MPT_ENUM(TypeUnixSocket), int),
	
	/* system pointer types (0x4 - 0x7) */
	pointer_type(MPT_ENUM(TypeFilePtr)),
	pointer_type(MPT_ENUM(TypeAddressPtr)),
	
	/* basic pointer types (0x8 - 0xf) */
	pointer_type(MPT_ENUM(TypeNodePtr)),
	pointer_type(MPT_ENUM(TypeReplyDataPtr)),
	
	/* basic value types (0x18 - 0x1f) */
	basic_type(MPT_ENUM(TypeValFmt),    MPT_STRUCT(value_format)),
	basic_type(MPT_ENUM(TypeValue),     MPT_STRUCT(value)),
	basic_type(MPT_ENUM(TypeProperty),  MPT_STRUCT(property)),
	basic_type(MPT_ENUM(TypeCommand),   MPT_STRUCT(command))
},
scalar_sizes[] = {
	/* basic printable types */
	basic_type('c', char),
	
	basic_type('b', int8_t),
	basic_type('y', uint8_t),
	
	basic_type('n', int16_t),
	basic_type('q', uint16_t),
	
	basic_type('i', int32_t),
	basic_type('u', uint32_t),
	
	basic_type('x', int64_t),
	basic_type('t', uint64_t),
	
	/* long type maps to equivalent size integer */
	
	basic_type('f', float),
	basic_type('d', double),
#ifdef _MPT_FLOAT_EXTENDED_H
	basic_type('e', long double),
#endif
	/* string types */
	pointer_type('s'),
};

static const struct {
	const char *name;
	int type;
} core_interfaces[] = {
	{ "convertable", MPT_ENUM(TypeConvertablePtr) },
	/* output interfaces */
	{ "logger",      MPT_ENUM(TypeLoggerPtr)      },
	{ "reply",       MPT_ENUM(TypeReplyPtr)       },
	{ "output",      MPT_ENUM(TypeOutputPtr)      },
	/* config interface types */
	{ "object",      MPT_ENUM(TypeObjectPtr)      },
	{ "config",      MPT_ENUM(TypeConfigPtr)      },
	/* input interface types */
	{ "iterator",    MPT_ENUM(TypeIteratorPtr)    },
	{ "collection",  MPT_ENUM(TypeCollectionPtr)  },
	/* other interfaces */
	{ "solver",      MPT_ENUM(TypeSolverPtr)      },
};

struct generic_traits_chunk
{
	const MPT_STRUCT(type_traits) *traits[30];
	struct generic_traits_chunk *next;
	uint8_t used;
};
struct named_traits_chunk
{
	MPT_STRUCT(named_traits) *traits[30];
	struct named_traits_chunk *next;
	uint8_t used;
};

static MPT_STRUCT(type_traits) *core_types = 0;
static MPT_STRUCT(type_traits) *scalar_types = 0;
static MPT_STRUCT(type_traits) *iovec_types = 0;

static MPT_STRUCT(type_traits) *dynamic_types = 0;
static int dynamic_pos = 0;

static MPT_STRUCT(named_traits) **interface_types = 0;
static int interface_pos = 0;

static struct named_traits_chunk *meta_types = 0;
static struct generic_traits_chunk *generic_types = 0;

static const MPT_STRUCT(type_traits) pointer_traits = MPT_TYPETRAIT_INIT(sizeof(void *));

/* core type resources */
static void _core_fini(void) {
	free(core_types);
	core_types = 0;
}
static void _core_init(void) {
	core_types = calloc(sizeof(*core_types), MPT_ENUM(_TypeCoreSize));
	size_t i;
	for (i = 0; i < MPT_arrsize(core_sizes); i++) {
		int pos = core_sizes[i].type;
		*((size_t *) &core_types[pos].size) = core_sizes[i].size;
	}
	atexit(_core_fini);
}
/* scalar type resources */
static void _scalar_fini(void) {
	free(scalar_types);
	scalar_types = 0;
}
static void _scalar_init(void) {
	scalar_types = calloc(sizeof(*scalar_types), MPT_ENUM(_TypeScalarSize));
	size_t i;
	for (i = 0; i < MPT_arrsize(scalar_sizes); i++) {
		int pos = scalar_sizes[i].type - MPT_ENUM(_TypeScalarBase);
		*((size_t *) &scalar_types[pos].size) = scalar_sizes[i].size;
	}
	atexit(_scalar_fini);
}
/* vector type resources */
static void _iovec_fini(void) {
	free(iovec_types);
	iovec_types = 0;
}
static void _iovec_init(void) {
	iovec_types = calloc(sizeof(*iovec_types), MPT_ENUM(_TypeVectorSize));
	size_t i;
	for (i = 0; i < MPT_arrsize(scalar_sizes); i++) {
		int pos = scalar_sizes[i].type - MPT_ENUM(_TypeScalarBase);
		*((size_t *) &iovec_types[pos].size) = sizeof(struct iovec);
	}
	atexit(_iovec_fini);
}
/* dynamic basic type resources */
static void _dynamic_fini(void) {
	free(dynamic_types);
	dynamic_types = 0;
	dynamic_pos = 0;
}
/* metatype pointer resources */
static void _meta_fini(void) {
	struct named_traits_chunk *group = meta_types;
	meta_types = 0;
	while (group) {
		struct named_traits_chunk *curr = group;
		int i;
		group = group->next;
		for (i = 0; i < curr->used; i++) {
			MPT_STRUCT(named_traits) *entry = curr->traits[i];
			free(entry);
		}
		free(curr);
	}
}
static void _meta_init(void) {
	MPT_STRUCT(named_traits) *base;
	
	if (!(meta_types = malloc(sizeof(*meta_types)))) {
		return;
	}
	meta_types->used = 0;
	atexit(_meta_fini);
	
	if (!(base = malloc(sizeof(*base) + sizeof(pointer_traits)))) {
		return;
	}
	*((const void **) &base->traits) = memcpy(base + 1, &pointer_traits, sizeof(pointer_traits));
	*((const char **) &base->name) = "metatype";
	*((uintptr_t *) &base->type) = MPT_ENUM(_TypeMetaPtrBase);
	
	meta_types->traits[0] = base;
	meta_types->next = 0;
	meta_types->used = 1;
}
/* interface resources */
static void _interfaces_fini(void) {
	int i;
	for (i = 0; i < interface_pos; i++) {
		free(interface_types[i]);
	}
	free(interface_types);
	interface_types = 0;
	interface_pos = 0;
}
static void _interfaces_init(void) {
	size_t i;
	if (!(interface_types = calloc(MPT_ENUM(_TypeInterfaceSize), sizeof(*interface_types)))) {
		return;
	}
	for (i = 0; i < MPT_arrsize(core_interfaces); i++) {
		MPT_STRUCT(named_traits) *elem = interface_types[i];
		
		if (elem || !(elem = malloc(sizeof(*elem) + sizeof(pointer_traits)))) {
			continue;
		}
		*((const void **) &elem->traits) = memcpy(elem + 1, &pointer_traits, sizeof(elem->traits));
		*((const char **) &elem->name) = core_interfaces[i].name;
		*((uintptr_t *) &elem->type) = core_interfaces[i].type;
		
		interface_types[i] = elem;
	}
	interface_pos = MPT_ENUM(_TypeInterfaceAdd) - MPT_ENUM(_TypeInterfaceBase);
	
	atexit(_interfaces_fini);
}
/* clean up generic types */
static void _generic_types_fini()
{
	struct generic_traits_chunk *group = generic_types;
	generic_types = 0;
	while (group) {
		struct generic_traits_chunk *curr = group;
		group = group->next;
		free(curr);
	}
}

/*!
 * \ingroup mptTypes
 * \brief get traits for registered type
 * 
 * Get operations and size of builtin or registered user type.
 * 
 * \param type  type ID code
 * 
 * \return traits for registered type
 */
extern const MPT_STRUCT(type_traits) *mpt_type_traits(int type)
{
	const struct generic_traits_chunk *group;
	
	/* bad type value */
	if (type < 0) {
		return 0;
	}
	/* builtin scalar types */
	if (type < MPT_ENUM(_TypeCoreSize)) {
		if (!core_types) {
			_core_init();
		}
		return core_types[type].size ? &core_types[type] : 0;
	}
	/* generic/typed vector */
	if (MPT_type_isScalar(type)) {
		if (!scalar_types) {
			_scalar_init();
		}
		type -= MPT_ENUM(_TypeScalarBase);
		return scalar_types[type].size ? &scalar_types[type] : 0;
	}
	/* generic/typed vector */
	if (MPT_type_isVector(type)) {
		if (!iovec_types) {
			_iovec_init();
		}
		type -= MPT_ENUM(_TypeVectorBase);
		return iovec_types[type].size ? &iovec_types[type] : 0;
	}
	/* interface type */
	if (MPT_type_isInterface(type)) {
		const MPT_STRUCT(named_traits) *it = mpt_interface_traits(type);
		return it ? it->traits : 0;
	}
	
	if (MPT_type_isDynamic(type)) {
		int pos = type - MPT_ENUM(_TypeDynamicBase);
		if (pos >= dynamic_pos) {
			return 0;
		}
		return dynamic_types + pos;
	}
	
	switch (type) {
		case MPT_ENUM(TypeArray):
			return mpt_array_traits();
		case MPT_ENUM(TypeMetaRef):
			return mpt_meta_reference_traits();
		default:;
	}
	
	if (MPT_type_isMetaPtr(type)) {
		const MPT_STRUCT(named_traits) *it = mpt_metatype_traits(type);
		return it ? it->traits : 0;
	}
	
	type -= MPT_ENUM(_TypeValueAdd);
	group = generic_types;
	while (group) {
		if (type < group->used) {
			return group->traits[type];
		}
		type -= MPT_arrsize(group->traits);
		group = group->next;
	}
	
	return 0;
}

/*!
 * \ingroup mptTypes
 * \brief get interface name
 * 
 * Get name for builtin or previously registered interface.
 * 
 * \return name for interface ID
 */
extern const MPT_STRUCT(named_traits) *mpt_interface_traits(int type)
{
	const MPT_STRUCT(named_traits) *elem;
	
	if (type > MPT_ENUM(_TypeInterfaceMax)
	    || type < MPT_ENUM(_TypeInterfaceBase)) {
		errno = EINVAL;
		return 0;
	}
	if (!interface_types) {
		_interfaces_init();
	}
	type -= MPT_ENUM(_TypeInterfaceBase);
	
	if (type > interface_pos) {
		errno = EAGAIN;
		return 0;
	}
	if ((elem = interface_types[type])) {
		return elem;
	}
	errno = EAGAIN;
	return 0;
}

/*!
 * \ingroup mptTypes
 * \brief get interface name
 * 
 * Get name for builtin or previously registered interface.
 * 
 * \return name for interface ID
 */
extern const MPT_STRUCT(named_traits) *mpt_metatype_traits(int type)
{
	const struct named_traits_chunk *ext;
	
	if (type > MPT_ENUM(_TypeMetaPtrMax)
	 || type < MPT_ENUM(_TypeMetaPtrBase)) {
		errno = EINVAL;
		return 0;
	}
	if (!meta_types) {
		_meta_init();
	}
	type -= MPT_ENUM(_TypeMetaPtrBase);
	
	ext = meta_types;
	while (ext) {
		if (type < ext->used) {
			const MPT_STRUCT(named_traits) *elem = (void *) ext->traits[type];
			if (!elem) {
				errno = EINVAL;
			}
			return elem;
		}
		type -= MPT_arrsize(ext->traits);
		ext = ext->next;
	}
	errno = EAGAIN;
	return 0;
}

/*!
 * \ingroup mptTypes
 * \brief get type for name
 * 
 * Get traits for previously registered named type.
 * Metatype entries take precedence over interfaces.
 * 
 * \return type ID for registered name
 */
extern const MPT_STRUCT(named_traits) *mpt_named_traits(const char *name, int len)
{
	const struct named_traits_chunk *ext;
	
	int i;
	
	if (!name || !len || !*name) {
		errno = EINVAL;
		return 0;
	}
	/* exact length match for names */
	if (len >= 0) {
		if (!(ext = meta_types)) {
			_meta_init();
			ext = meta_types;
		}
		while (ext) {
			for (i = 0; i < ext->used; i++) {
				const MPT_STRUCT(named_traits) *elem = ext->traits[i];
				if (elem->name
				 && (len == (int) strlen(elem->name))
				 && !strncmp(name, elem->name, len)) {
					return elem;
				}
			}
			ext = ext->next;
		}
		if (!interface_types) {
			_interfaces_init();
		}
		for (i = 0; i < interface_pos; i++) {
			const MPT_STRUCT(named_traits) *elem;
			if ((elem = interface_types[i])
			 && elem->name
			 && (len == (int) strlen(elem->name))
			 && !strncmp(name, elem->name, len)) {
				return elem;
			}
		}
		errno = EINVAL;
		return 0;
	}
	/* full names without length limit */
	if (!(len = strlen(name))) {
		errno = EINVAL;
		return 0;
	}
	/* resolve shortnames */
	if (!strcmp(name, "log")) {
		name = "logger";
	}
	else if (!strcmp(name, "iter")) {
		name = "iterator";
	}
	else if (!strcmp(name, "out")) {
		name = "output";
	}
	else if (!strcmp(name, "meta")) {
		name = "metatype";
	}
	if (!(ext = meta_types)) {
		_meta_init();
		ext = meta_types;
	}
	while (ext) {
		for (i = 0; i < ext->used; i++) {
			const MPT_STRUCT(named_traits) *elem = ext->traits[i];
			if (elem->name && !strcmp(name, elem->name)) {
				return elem;
			}
		}
		ext = ext->next;
	}
	if (!interface_types) {
		_interfaces_init();
	}
	for (i = 0; i < interface_pos; i++) {
		const MPT_STRUCT(named_traits) *elem = interface_types[i];
		if (elem && elem->name && !strcmp(name, elem->name)) {
			return elem;
		}
	}
	errno = EINVAL;
	return 0;
}

/*!
 * \ingroup mptTypes
 * \brief register generic type
 * 
 * Register new type with free-form traits.
 * 
 * \return type code of new generic type
 */
extern int mpt_type_add(const MPT_STRUCT(type_traits) *traits)
{
	struct generic_traits_chunk *curr;
	int pos = MPT_ENUM(_TypeValueAdd);
	
	/* must have valid size */
	if (!traits || !traits->size) {
		return MPT_ERROR(BadArgument);
	}
	/* create initial traits chunk */
	if (!(curr = generic_types)) {
		if (!(curr = malloc(sizeof(*generic_types)))) {
			return MPT_ERROR(BadOperation);
		}
		curr->next = 0;
		curr->used = 0;
		generic_types = curr;
		atexit(_generic_types_fini);
	}
	/* find available traits chunk entry */
	while (curr->used == MPT_arrsize(curr->traits)) {
		struct generic_traits_chunk *next;
		pos += MPT_arrsize(curr->traits);
		/* append new empty chunk */
		if (!(next = curr->next)) {
			if (pos > MPT_ENUM(_TypeValueMax)) {
				return MPT_ERROR(BadType);
			}
			if (!(next = malloc(sizeof(*next)))) {
				return MPT_ERROR(BadOperation);
			}
			next->next = 0;
			next->used = 0;
			curr->next = next;
		}
		curr = next;
	}
	pos += curr->used;
	if (pos > MPT_ENUM(_TypeValueMax)) {
		return MPT_ERROR(BadType);
	}
	curr->traits[curr->used++] = traits;
	return pos;
}

/*!
 * \ingroup mptTypes
 * \brief register new type
 * 
 * register new user type to use with mpt_type_traits()
 * 
 * \param traits  operations and size for new type
 * 
 * \return type code of new user type
 */
extern int mpt_type_basic_add(size_t size)
{
	if (!size) {
		size = sizeof(void *);
	}
	if (!dynamic_types) {
		dynamic_types = calloc(MPT_ENUM(_TypeDynamicSize), sizeof(*dynamic_types));
		if (!dynamic_types) {
			return MPT_ERROR(BadOperation);
		}
		atexit(_dynamic_fini);
	}
	if (dynamic_pos < MPT_ENUM(_TypeDynamicSize)) {
		const MPT_STRUCT(type_traits) traits = MPT_TYPETRAIT_INIT(size);
		memcpy(&dynamic_types[dynamic_pos], &traits, sizeof(*dynamic_types));
		return MPT_ENUM(_TypeDynamicBase) + dynamic_pos++;
	}
	
	return MPT_ERROR(MissingBuffer);
}

/*!
 * \ingroup mptTypes
 * \brief register metatype
 * 
 * Register name for new global metatype.
 * 
 * \return unique type code
 */
extern const MPT_STRUCT(named_traits) *mpt_type_metatype_add(const char *name)
{
	struct named_traits_chunk *ext;
	MPT_STRUCT(named_traits) *elem;
	MPT_STRUCT(type_traits) *traits;
	size_t nlen = 0;
	int pos;
	
	if (!(ext = meta_types)) {
		_meta_init();
		ext = meta_types;
	}
	
	if (name) {
		nlen = strlen(name);
		if (nlen++ < 4) {
			errno = EINVAL;
			return 0;
		}
		while (ext) {
			int i, max;
			for (i = 0, max = ext->used; i < max; i++) {
				elem = ext->traits[i];
				if (elem->name && !strcmp(elem->name, name)) {
					errno = EINVAL;
					return 0;
				}
			}
			ext = ext->next;
		}
		ext = meta_types;
	}
	pos = MPT_ENUM(_TypeMetaPtrBase);
	
	while (ext->used == MPT_arrsize(ext->traits)) {
		struct named_traits_chunk *next;
		pos += MPT_arrsize(ext->traits);
		if (pos > MPT_ENUM(_TypeMetaPtrMax)) {
			errno = ENOMEM;
			return 0;
		}
		if (!(next = ext->next)) {
			if (!(next = malloc(sizeof(*next)))) {
				return 0;
			}
			next->next = 0;
			next->used = 0;
			ext->next = next;
		}
		ext = next;
	}
	pos += ext->used;
	if (pos > MPT_ENUM(_TypeMetaPtrMax)) {
		errno = ENOMEM;
		return 0;
	}
	if (!(elem = malloc(sizeof(*elem) + sizeof(pointer_traits) + nlen))) {
		return 0;
	}
	traits = memcpy(elem + 1, &pointer_traits, sizeof(pointer_traits));
	*((const void **) &elem->traits) = traits;
	*((const char **) &elem->name) = nlen ? memcpy(traits + 1, name, nlen) : 0;
	*((uintptr_t *) &elem->type) = pos;
	
	ext->traits[ext->used++] = elem;
	
	return elem;
}
/*!
 * \ingroup mptTypes
 * \brief register object
 * 
 * Register new interface type to use with mpt_valsize()
 * and other internal type operations.
 * 
 * \return type code of new object type
 */
extern const MPT_STRUCT(named_traits) *mpt_type_interface_add(const char *name)
{
	MPT_STRUCT(named_traits) *elem;
	MPT_STRUCT(type_traits) *traits;
	size_t nlen = 0;
	if (interface_pos >= (MPT_ENUM(_TypeInterfaceSize))) {
		errno = ENOMEM;
		return 0;
	}
	
	if (!interface_types) {
		_interfaces_init();
	}
	
	if (name) {
		int i;
		for (i = 0; i < interface_pos; i++) {
			elem = interface_types[i];
			if (elem && elem->name && !strcmp(elem->name, name)) {
				errno = EINVAL;
				return 0;
			}
		}
		nlen = strlen(name);
		if (nlen++ < 4) {
			errno = EINVAL;
			return 0;
		}
	}
	if (!(elem = malloc(sizeof(*elem) + sizeof(pointer_traits) + nlen))) {
		return 0;
	}
	traits = memcpy(elem + 1, &pointer_traits, sizeof(pointer_traits));
	*((const void **) &elem->traits) = traits;
	*((const char **) &elem->name) = nlen ? memcpy(traits + 1, name, nlen) : 0;
	*((uintptr_t*) &elem->type) = MPT_ENUM(_TypeInterfaceBase) + interface_pos;
	
	interface_types[interface_pos++] = elem;
	
	return elem;
}
