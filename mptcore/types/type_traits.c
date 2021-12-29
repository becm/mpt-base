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

#include "mptplot/layout.h"

#include "types.h"

#define basic_type(t, s) { MPT_TYPETRAIT_INIT(s), (t) }
#define pointer_type(t)  { MPT_TYPETRAIT_INIT(sizeof(void *)), (t) }

static void _array_fini(void *ptr)
{
	MPT_STRUCT(array) *arr = ptr;
	MPT_STRUCT(buffer) *buf = arr->_buf;
	if (buf) {
		buf->_vptr->unref(buf);
	}
}
static int _array_init(void *ptr, const void *src)
{
	MPT_STRUCT(array) *arr = ptr;
	MPT_STRUCT(buffer) *buf = 0;
	if (src && (buf = ((const MPT_STRUCT(array) *) src)->_buf)) {
		if (!buf->_vptr->addref(buf)) {
			return MPT_ERROR(BadOperation);
		}
	}
	arr->_buf = buf;
	return buf ? 1 : 0;
}

static void _mref_fini(void *ptr)
{
	MPT_INTERFACE(metatype) *mt = *((void **) ptr);
	if (mt) {
		mt->_vptr->unref(mt);
	}
}
static int _mref_init(void *ptr, const void *src)
{
	MPT_INTERFACE(metatype) *mt = 0;
	if (src && (mt = *((MPT_INTERFACE(metatype) * const *) src))) {
		if (!mt->_vptr->addref(mt)) {
			return MPT_ERROR(BadOperation);
		}
	}
	*((void **) ptr) = mt;
	return mt ? 1 : 0;
}

static const struct {
	const MPT_STRUCT(type_traits) traits;
	int type;
} static_ptypes[] = {
	/* system types (0x1 - 0x3) */
	basic_type(MPT_ENUM(TypeUnixSocket), sizeof(int)),
	
	/* system pointer types (0x4 - 0x7) */
	pointer_type(MPT_ENUM(TypeFilePtr)),
	pointer_type(MPT_ENUM(TypeAddressPtr)),
	
	/* basic pointer types (0x8 - 0xf) */
	pointer_type(MPT_ENUM(TypeNodePtr)),
	pointer_type(MPT_ENUM(TypeReplyDataPtr)),
	
	/* layout value types (0x10 - 0x13) */
	basic_type(MPT_ENUM(TypeColor),    sizeof(MPT_STRUCT(color))),
	basic_type(MPT_ENUM(TypeLineAttr), sizeof(MPT_STRUCT(lineattr))),
	basic_type(MPT_ENUM(TypeLine),     sizeof(MPT_STRUCT(line))),
	
	/* layout value types (0x14 - 0x17) */
	pointer_type(MPT_ENUM(TypeAxisPtr)),
	pointer_type(MPT_ENUM(TypeTextPtr)),
	pointer_type(MPT_ENUM(TypeWorldPtr)),
	pointer_type(MPT_ENUM(TypeGraphPtr)),
	
	/* basic value types (0x18 - 0x1f) */
	basic_type(MPT_ENUM(TypeValFmt),    sizeof(MPT_STRUCT(value_format))),
	basic_type(MPT_ENUM(TypeValue),     sizeof(MPT_STRUCT(value))),
	basic_type(MPT_ENUM(TypeProperty),  sizeof(MPT_STRUCT(property))),
	basic_type(MPT_ENUM(TypeCommand),   sizeof(MPT_STRUCT(command))),
	
	/* basic printable types */
	basic_type('c', sizeof(char)),
	
	basic_type('b', sizeof(int8_t)),
	basic_type('y', sizeof(uint8_t)),
	
	basic_type('n', sizeof(int16_t)),
	basic_type('q', sizeof(uint16_t)),
	
	basic_type('i', sizeof(int32_t)),
	basic_type('u', sizeof(uint32_t)),
	
	basic_type('x', sizeof(int64_t)),
	basic_type('t', sizeof(uint64_t)),
	
	/* long type maps to equivalent size integer */
	
	basic_type('f', sizeof(float)),
	basic_type('d', sizeof(double)),
#ifdef _MPT_FLOAT_EXTENDED_H
	basic_type('e', sizeof(long double)),
#endif
	/* string types */
	pointer_type('s'),
	
	/* reference types */
	{ { _array_init, _array_fini, sizeof(MPT_STRUCT(array))  }, MPT_ENUM(TypeArray) },
	{ { _mref_init,  _mref_fini,  sizeof(MPT_STRUCT(void *)) }, MPT_ENUM(TypeMetaRef) },
};

static const struct {
	const char *name;
	int type;
} core_interfaces[] = {
	/* option interface types */
	{ "object",   MPT_ENUM(TypeObjectPtr)   },
	{ "config",   MPT_ENUM(TypeConfigPtr)   },
	/* input interface types */
	{ "iterator", MPT_ENUM(TypeIteratorPtr) },
	/* output interfaces */
	{ "logger",   MPT_ENUM(TypeLoggerPtr)   },
	{ "reply",    MPT_ENUM(TypeReplyPtr)    },
	{ "output",   MPT_ENUM(TypeOutputPtr)   },
	/* other interfaces */
	{ "solver",   MPT_ENUM(TypeSolverPtr)   },
};

struct type_entry
{
	MPT_STRUCT(type_traits) traits;
	const char *name;
};

static MPT_STRUCT(type_traits)  iovec_types[MPT_ENUM(_TypeVectorSize)];

static const MPT_STRUCT(type_traits) **dynamic_types = 0;
static int dynamic_pos = 0;

static struct type_entry **interface_types = 0;
static int interface_pos = 0;

static struct type_entry **meta_types = 0;
static int meta_pos = 0;


static void _dynamic_fini(void) {
	free(dynamic_types);
	dynamic_types = 0;
	dynamic_pos = 0;
}

static void _meta_fini(void) {
	int i;
	for (i = 0; i < meta_pos; i++) {
		struct type_entry *traits = meta_types[i];
		if (traits) {
			free(traits);
		}
	}
	free(meta_types);
	meta_types = 0;
}
static void _meta_init(void) {
	struct type_entry *base;
	
	if (!(meta_types = calloc(MPT_ENUM(_TypeMetaPtrSize), sizeof(*meta_types)))) {
		return;
	}
	if ((base = malloc(sizeof(*base)))) {
		static const char base_name[] = "metatype";
		base->traits.init = 0;
		base->traits.fini = 0;
		base->traits.size = sizeof(void *);
		base->name = base_name;
	}
	meta_types[meta_pos++] = base;
	
	atexit(_meta_fini);
}

static void _interfaces_fini(void) {
	int i;
	for (i = 0; i < interface_pos; i++) {
		struct type_entry *traits = interface_types[i];
		if (traits) {
			free(traits);
		}
	}
	free(interface_types);
	interface_types = 0;
}
static void _interfaces_init(void) {
	size_t i;
	if (!(interface_types = calloc(MPT_ENUM(_TypeInterfaceSize), sizeof(*interface_types)))) {
		return;
	}
	for (i = 0; i < MPT_arrsize(core_interfaces); i++) {
		int pos = core_interfaces[i].type - MPT_ENUM(_TypeInterfaceBase);
		struct type_entry *elem = interface_types[pos];
		
		if (elem || !(elem = malloc(sizeof(*elem)))) {
			continue;
		}
		elem->traits.init = 0;
		elem->traits.fini = 0;
		elem->traits.size = sizeof(void *);
		elem->name = core_interfaces[i].name;
		
		interface_types[pos] = elem;
	}
	interface_pos = MPT_ENUM(_TypeInterfaceAdd) - MPT_ENUM(_TypeInterfaceBase);
	
	atexit(_interfaces_fini);
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
	/* bad type value */
	if (type < 0) {
		return 0;
	}
	/* builtin scalar types */
	if (type < 0x20 || MPT_type_isScalar(type)) {
		size_t i;
		/* basic type */
		for (i = 0; i < MPT_arrsize(static_ptypes); ++i) {
			if (type == static_ptypes[i].type) {
				return &static_ptypes[i].traits;
			}
		}
		return 0;
	}
	/* generic/typed vector */
	if (MPT_type_isVector(type)) {
		MPT_STRUCT(type_traits) *traits = &iovec_types[type - MPT_ENUM(_TypeVectorBase)];
		if (!traits->size) {
			traits->init = 0;
			traits->fini = 0;
			traits->size = sizeof(struct iovec);
		}
		return traits;
	}
	/* interface type */
	if (MPT_type_isInterface(type)) {
		const struct type_entry *entry;
		
		type -= MPT_ENUM(_TypeInterfaceBase);
		if (!interface_types) {
			_interfaces_init();
		}
		entry = interface_types[type - MPT_ENUM(_TypeDynamicBase)];
		return entry ? &entry->traits : 0;
	}
	
	if (MPT_type_isDynamic(type)) {
		if (dynamic_types) {
			const MPT_STRUCT(type_traits) *traits = dynamic_types[type - MPT_ENUM(_TypeDynamicBase)];
			return (traits && traits->size) ? traits : 0;
		}
		return 0;
	}
	
	if (MPT_type_isMetaPtr(type)) {
		const struct type_entry *entry;
		
		if (!meta_types) {
			_meta_init();
		}
		entry = meta_types[type - MPT_ENUM(_TypeMetaPtrBase)];
		return entry ? &entry->traits : 0;
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
extern const char *mpt_interface_typename(int type)
{
	const struct type_entry *elem;
	
	if (type > MPT_ENUM(_TypeInterfaceMax)
	    || type < MPT_ENUM(_TypeInterfaceBase)) {
		errno = EINVAL;
		return 0;
	}
	if (!interface_types) {
		errno = EAGAIN;
		return 0;
	}
	
	if ((elem = interface_types[type - MPT_ENUM(_TypeInterfaceBase)])) {
		return elem->name ? elem->name : "";
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
extern const char *mpt_meta_typename(int type)
{
	const struct type_entry *elem;
	
	if (type > MPT_ENUM(_TypeMetaPtrMax)
	    || type < MPT_ENUM(_TypeMetaPtrBase)) {
		errno = EINVAL;
		return 0;
	}
	if (!meta_types) {
		errno = EAGAIN;
		return 0;
	}
	
	if ((elem = meta_types[type - MPT_ENUM(_TypeMetaPtrBase)])) {
		return elem->name ? elem->name : "";
	}
	errno = EAGAIN;
	return 0;
}

/*!
 * \ingroup mptTypes
 * \brief get type for name
 * 
 * Get name for previously registered type name.
 * Metatype entries take precedence over interfaces.
 * 
 * \return type ID for registered name
 */
extern int mpt_type_value(const char *name, int len)
{
	const struct type_entry *elem;
	int i;
	
	if (!name || !len || !*name) {
		return MPT_ERROR(BadArgument);
	}
	if (!meta_types) {
		_meta_init();
	}
	if (!interface_types) {
		_interfaces_init();
	}
	/* exact length match for names */
	if (len >= 0) {
		for (i = 0; i < meta_pos; i++) {
			if ((elem = meta_types[i])
			 && len == (int) strlen(elem->name)
			 && !strncmp(name, elem->name, len)) {
				return MPT_ENUM(_TypeMetaPtrBase) + i;
			}
		}
		for (i = 0; i < interface_pos; i++) {
			if ((elem = interface_types[i])
			 && len == (int) strlen(elem->name)
			 && !strncmp(name, elem->name, len)) {
				return MPT_ENUM(_TypeInterfaceBase) + i;
			}
		}
		return MPT_ERROR(BadValue);
	}
	/* full names without length limit */
	if (!(len = strlen(name))) {
		return MPT_ERROR(BadArgument);
	}
	for (i = 0; i < interface_pos; i++) {
		if ((elem = interface_types[i])
		 && !strncmp(name, elem->name, len)) {
			return MPT_ENUM(_TypeInterfaceBase) + i;
		}
	}
	/* shortnames */
	if (!strcmp(name, "log")) {
		return MPT_ENUM(TypeLoggerPtr);
	}
	if (!strcmp(name, "iter")) {
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (!strcmp(name, "out")) {
		return MPT_ENUM(TypeOutputPtr);
	}
	if (!strcmp(name, "meta")) {
		return MPT_ENUM(TypeMetaPtr);
	}
	return MPT_ERROR(BadValue);
	
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
extern int mpt_type_generic_new(const MPT_STRUCT(type_traits) *traits)
{
	if (!traits || !traits->size) {
		return MPT_ERROR(BadArgument);
	}
	if (!dynamic_types) {
		dynamic_types = calloc(MPT_ENUM(_TypeDynamicSize), sizeof(*dynamic_types));
		if (!dynamic_types) {
			return MPT_ERROR(BadOperation);
		}
		atexit(_dynamic_fini);
	}
	if (dynamic_pos < MPT_ENUM(_TypeDynamicSize)) {
		dynamic_types[dynamic_pos] = traits;
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
extern int mpt_type_meta_new(const char *name)
{
	struct type_entry *elem;
	size_t nlen = 0;
	
	if (meta_pos >= MPT_ENUM(_TypeMetaPtrSize)) {
		return MPT_ERROR(MissingBuffer);
	}
	
	if (name) {
		int i;
		for (i = 0; i < meta_pos; i++) {
			elem = meta_types[i];
			if (elem && elem->name && !strcmp(elem->name, name)) {
				return MPT_ERROR(BadValue);
			}
		}
		nlen = strlen(name);
		if (nlen++ < 4) {
			return MPT_ERROR(BadValue);
		}
	}
	
	if (!(elem = malloc(sizeof(*elem) + nlen))) {
		return MPT_ERROR(BadOperation);
	}
	elem->traits.init = 0;
	elem->traits.fini = 0;
	elem->traits.size = sizeof(void *);
	elem->name = name ? memcpy(elem + 1, name, nlen) : 0;
	
	
	if (!meta_types) {
		_meta_init();
	}
	meta_types[meta_pos] = elem;
	
	return MPT_ENUM(_TypeMetaPtrBase) + meta_pos;
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
extern int mpt_type_interface_new(const char *name)
{
	struct type_entry *elem;
	size_t nlen = 0;
	if (interface_pos >= (MPT_ENUM(_TypeInterfaceSize))) {
		return MPT_ERROR(MissingBuffer);
	}
	
	if (name) {
		int i;
		for (i = 0; i < interface_pos; i++) {
			elem = interface_types[i];
			if (elem && elem->name && !strcmp(elem->name, name)) {
				return MPT_ERROR(BadValue);
			}
		}
		nlen = strlen(name);
		if (nlen++ < 4) {
			return MPT_ERROR(BadArgument);
		}
	}
	if (!(elem = malloc(sizeof(*elem) + nlen))) {
		return MPT_ERROR(BadOperation);
	}
	elem->traits.init = 0;
	elem->traits.fini = 0;
	elem->traits.size = sizeof(void *);
	elem->name = nlen ? memcpy(elem + 1, name, nlen) : 0;
	
	
	if (!interface_types) {
		_interfaces_init();
	}
	interface_types[interface_pos] = elem;
	
	return MPT_ENUM(_TypeInterfaceBase) + interface_pos++;
}
