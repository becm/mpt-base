/*!
 * MPT core library
 *  dynamic type system
 */

#ifndef _MPT_TYPES_H
#define _MPT_TYPES_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

enum MPT_ENUM(Types)
{
	/* system types */
	MPT_ENUM(TypeUnixSocket)    = 0x1,   /* SOH */
	/* system pointer types */
	MPT_ENUM(TypeFilePtr)       = 0x4,   /* EOT */
	MPT_ENUM(TypeAddressPtr)    = 0x5,   /* ENQ */
	
	/* special pointer types */
	MPT_ENUM(TypeReplyDataPtr)  = 0x8,  /* BS */
	MPT_ENUM(TypeNodePtr)       = 0x9,  /* HT' */
	
	/* reserve 0x10..0x17 for layout types */
	
	/* format types (scalar) */
	MPT_ENUM(TypeValFmt)        = 0x18,  /* CAN */
	MPT_ENUM(TypeValue)         = 0x19,  /* EM  */
	MPT_ENUM(TypeProperty)      = 0x1a,  /* SUB */
	MPT_ENUM(TypeCommand)       = 0x1b,  /* ESC */
	
	/* range for generic base types */
	MPT_ENUM(_TypeVectorBase)    = 0x40,
	MPT_ENUM(_TypeVectorMax)     = 0x5a,
	MPT_ENUM(_TypeVectorSize)    = 0x20,
#define MPT_type_isVector(v)      ((v) >= MPT_ENUM(_TypeVectorBase) && (v) < MPT_ENUM(_TypeVectorMax))
#define MPT_type_toVector(v)      (MPT_type_isScalar(v) \
                                 ? (v) - MPT_ENUM(_TypeScalarBase) + MPT_ENUM(_TypeVectorBase) \
                                 : MPT_ERROR(BadType))
	MPT_ENUM(_TypeScalarBase)    = 0x60,
	MPT_ENUM(_TypeScalarMax)     = 0x7a,
	MPT_ENUM(_TypeScalarSize)    = 0x20,
#define MPT_type_isScalar(v)      ((v) >= MPT_ENUM(_TypeScalarBase) && (v) <= MPT_ENUM(_TypeScalarMax))
#define MPT_type_toScalar(v)      (MPT_type_isVector(v) \
                                 ? (v) - MPT_ENUM(_TypeVectorBase) + MPT_ENUM(_TypeScalarBase) \
                                 : MPT_ERROR(BadType))

	/* vector types ('@'..'Z') */
	MPT_ENUM(TypeVector)         = '@',  /* 0x40: generic data */
	/* scalar types ('a'..'z') */
	MPT_ENUM(TypeArray)          = 'a',   /* array content */
	MPT_ENUM(TypeMetaRef)        = 'm',   /* generic metatype reference */
	
	/* conversion interface */
	MPT_ENUM(TypeConvertablePtr) = 0x80,
	/* output interface types */
	MPT_ENUM(TypeLoggerPtr)      = 0x81,
	MPT_ENUM(TypeReplyPtr)       = 0x82,
	MPT_ENUM(TypeOutputPtr)      = 0x83,
	/* config interface types */
	MPT_ENUM(TypeObjectPtr)      = 0x84,
	MPT_ENUM(TypeConfigPtr)      = 0x85,
	/* collection interface types */
	MPT_ENUM(TypeIteratorPtr)    = 0x86,
	MPT_ENUM(TypeCollectionPtr)  = 0x87,
	/* external interface types */
	MPT_ENUM(TypeSolverPtr)      = 0x88,
	/* range for dynamic interfaces */
	MPT_ENUM(_TypeInterfaceBase) = 0x80,
	MPT_ENUM(_TypeInterfaceAdd)  = MPT_ENUM(_TypeInterfaceBase) + 0x10,
	MPT_ENUM(_TypeInterfaceMax)  = MPT_ENUM(_TypeInterfaceBase) + 0x3f,
	MPT_ENUM(_TypeInterfaceSize) = 0x40,
#define MPT_type_isInterface(v)   ((v) >= MPT_ENUM(_TypeInterfaceBase) && (v) <= MPT_ENUM(_TypeInterfaceMax))
	
	MPT_ENUM(_TypeDynamicBase)   = MPT_ENUM(_TypeInterfaceBase) + MPT_ENUM(_TypeInterfaceSize),
	MPT_ENUM(_TypeDynamicMax)    = 0xff,
	MPT_ENUM(_TypeDynamicSize)   = 0x100 - MPT_ENUM(_TypeDynamicBase),
#define MPT_type_isDynamic(v)     ((v) >= MPT_ENUM(_TypeDynamicBase) && (v) <= MPT_ENUM(_TypeDynamicMax))
	
	MPT_ENUM(_TypeMetaPtrBase)   = 0x100,
	MPT_ENUM(_TypeMetaPtrMax)    = 0x1ff,
	MPT_ENUM(_TypeMetaPtrSize)   = 0x100,
#define MPT_type_isMetaPtr(v)     ((v) >= MPT_ENUM(_TypeMetaPtrBase) && (v) <= MPT_ENUM(_TypeMetaPtrMax))
	MPT_ENUM(TypeMetaPtr)        = MPT_ENUM(_TypeMetaPtrBase)
};

MPT_STRUCT(type_traits)
{
#ifdef __cplusplus
	inline type_traits(size_t _size, void (*_fini)(void *) = 0, int (*_init)(void *, const void *) = 0)
		: init(_init), fini(_fini), size(_size)
	{ }
	static int add_basic(size_t);
	static int add(const type_traits &);
	
	static const type_traits *get(int);
#else
# define MPT_TYPETRAIT_INIT(t)  { 0, 0, (t) }
#endif
	int  (*init)(void *, const void *);
	void (*fini)(void *);
	size_t size;
};


__MPT_EXTDECL_BEGIN

/* get type position from data description */
extern int mpt_position(const uint8_t *, int);
/* get position offset from data description */
extern int mpt_offset(const uint8_t *, int);


/* query type mappings */
extern const MPT_STRUCT(type_traits) *mpt_type_traits(int);

/* id for registered named type */
extern int mpt_type_value(const char *, int);
/* get name registered type */
extern const char *mpt_meta_typename(int);
extern const char *mpt_interface_typename(int);
/* register additional types */
extern int mpt_type_meta_new(const char *);
extern int mpt_type_interface_new(const char *);
extern int mpt_type_generic_new(const MPT_STRUCT(type_traits) *);

/* type alias for symbol description */
extern int mpt_alias_typeid(const char *, const char **__MPT_DEFPAR(0));


/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);
/* read from value */
extern int mpt_value_read(MPT_STRUCT(value) *, const char *, void *);

__MPT_EXTDECL_END

#ifdef __cplusplus

inline uint8_t basetype(int org) {
	if (org <= 0) {
		return 0;
	}
	if (org <= _TypeDynamicMax) {
		return org;
	}
	if (org <= _TypeMetaPtrMax) {
		return TypeConvertablePtr;
	}
	return 0;
}

template<typename T>
class type_properties
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits traits(sizeof(T), _fini, _init);
		return &traits;
	}
	
	static int id() {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		return _valtype = type_traits::add(*traits());
	}
private:
	type_properties();
	
	static int _init(void *ptr, const void *src) {
		if (!src) {
			new (ptr) T();
		}
		else {
			new (ptr) T(*static_cast<const T *>(src));
		}
		return 0;
	}
	static void _fini(void *ptr) {
		static_cast<T *>(ptr)->~T();
	}
};

template<typename T>
class type_properties<T *>
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits traits(sizeof(T *));
		return &traits;
	}
	
	static int id() {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		return _valtype = type_traits::add(*traits());
	}
private:
	type_properties();
};

template<typename T>
class type_properties<span<const T> >
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits *traits = 0;
		if (traits) {
			return traits;
		}
		int type = id();
		if (type < 0 || !(traits = type_traits::get(type))) {
			traits = get_traits();
		}
		return traits;
	}
	
	static int id() {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		/* use vector IDs for builtin types */
		int type = type_properties<T>::id();
		if ((type = MPT_type_toVector(type)) > 0) {
			return _valtype = type;
		}
		return _valtype = type_traits::add(*get_traits());
	}
private:
	static const type_traits *get_traits() {
		static const struct type_traits span_traits(sizeof(span<T>));
		return &span_traits;
	}
	type_properties();
};

template<typename T>
bool assign(const T &from, int type, void *ptr) {
	if (type != type_properties<T>::id()) {
		return false;
	}
	if (ptr) {
		*static_cast<T *>(ptr) = from;
	}
	return true;
}

template<typename T>
inline T *typecast(convertable &src) {
	T *ptr = 0;
	int type = type_properties<T *>::id();
	if ((type > 0)
	 && (src.convert(type, &ptr) < 0)) {
		ptr = 0;
	}
	return ptr;
}


/* specialize convertable string cast */
template <> inline const char *typecast<const char>(convertable &src) {
	return src.string();
}

/* floating point values */
template<> inline __MPT_CONST_TYPE int type_properties<float>::id()       { return 'f'; }
template<> inline __MPT_CONST_TYPE int type_properties<double>::id()      { return 'd'; }
template<> inline __MPT_CONST_TYPE int type_properties<long double>::id() { return 'e'; }
/* integer values */
template<> inline __MPT_CONST_TYPE int type_properties<int8_t>::id()  { return 'b'; }
template<> inline __MPT_CONST_TYPE int type_properties<int16_t>::id() { return 'n'; }
template<> inline __MPT_CONST_TYPE int type_properties<int32_t>::id() { return 'i'; }
template<> inline __MPT_CONST_TYPE int type_properties<int64_t>::id() { return 'x'; }
/* unsigned values */
template<> inline __MPT_CONST_TYPE int type_properties<uint8_t>::id()  { return 'y'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint16_t>::id() { return 'q'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint32_t>::id() { return 'u'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint64_t>::id() { return 't'; }
/* string data */
template<> inline __MPT_CONST_TYPE int type_properties<char>::id() { return 'c'; }
template<> inline __MPT_CONST_TYPE int type_properties<const char *>::id() { return 's'; }


/* floating point values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<float>::traits()       { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<double>::traits()      { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<long double>::traits() { return type_traits::get(id()); }
/* integer values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<int8_t>::traits()  { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int16_t>::traits() { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int32_t>::traits() { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int64_t>::traits() { return type_traits::get(id()); }
/* unsigned values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint8_t>::traits()  { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint16_t>::traits() { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint32_t>::traits() { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint64_t>::traits() { return type_traits::get(id()); }
/* string data */
template<> inline const MPT_STRUCT(type_traits) *type_properties<char>::traits() { return type_traits::get(id()); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<const char *>::traits() { return type_traits::get(id()); }


template<> inline __MPT_CONST_TYPE int type_properties<value>::id() {
	return TypeValue;
}
template<> inline const MPT_STRUCT(type_traits) *type_properties<value>::traits() {
	return type_traits::get(id());
}

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#ifdef __cplusplus
#endif

#endif /* _MPT_TYPES_H */
