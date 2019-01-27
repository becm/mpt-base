/*!
 * MPT core library
 *  dynamic type system
 */

#ifndef _MPT_TYPES_H
#define _MPT_TYPES_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int typeinfo<value>::id()
{
	return value::Type;
}
#endif


__MPT_EXTDECL_BEGIN

/* get type position from data description */
extern int mpt_position(const uint8_t *, int);
/* get position offset from data description */
extern int mpt_offset(const uint8_t *, int);
/* get size for registered types */
extern ssize_t mpt_valsize(int);

/* id for registered named type */
extern int mpt_type_value(const char *, int);
/* get name registered type */
extern const char *mpt_meta_typename(int);
extern const char *mpt_interface_typename(int);
/* register extended types */
extern int mpt_type_meta_new(const char *);
extern int mpt_type_interface_new(const char *);
extern int mpt_type_generic_new(void);

/* add user scalar or pointer type */
extern int mpt_valtype_add(size_t);

/* type alias for symbol description */
extern int mpt_alias_typeid(const char *, const char **__MPT_DEFPAR(0));


/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);
/* read from value */
extern int mpt_value_read(MPT_STRUCT(value) *, const char *, void *);

__MPT_EXTDECL_END

#ifdef __cplusplus

extern int make_id();

inline __MPT_CONST_EXPR int to_pointer_id(int from)
{
	return (from < 0)
		? BadValue
		: ((from > _TypeGenericMax)
			? BadType
			: _TypePointerBase + from);
}
inline __MPT_CONST_EXPR int to_reference_id(int from)
{
	return (from < 0)
		? BadValue
		: ((from > _TypeGenericMax)
			? BadType
			: _TypeReferenceBase + from);
}
inline __MPT_CONST_EXPR int to_item_id(int from)
{
	return (from < 0)
		? BadValue
		: ((from > _TypeGenericMax)
			? BadType
			: _TypeItemBase + from);
}

inline __MPT_CONST_EXPR int to_span_id(int from)
{
	return (from < 0)
		? BadValue
		: ((from > _TypeGenericMax)
			? BadType
			: ((MPT_type_isScalar(from) || MPT_type_isExtended(from))
				? from + _TypeVectorBase - _TypeScalarBase
				: _TypeSpanBase + from));
}

inline __MPT_CONST_EXPR uint8_t basetype(int id)
{
	return (id >= (MPT_ENUM(_TypeMetaBase) + MPT_ENUM(_TypeReferenceBase))
	     && id <= (MPT_ENUM(_TypeMetaMax) + MPT_ENUM(_TypeReferenceBase)))
		? TypeMetaRef
		: ((id < 0 || id > 0xff) ? 0 : id);
}
template <typename T>
class typeinfo<T *>
{
protected:
	typeinfo();
public:
	static int id()
	{
		return to_pointer_id(typeinfo<T>::id());
	}
};

/* vector-type auto-cast for constant base types */
template <typename T>
class typeinfo<span<T> >
{
protected:
	typeinfo();
public:
	static int id()
	{
		static int _id;
		if (!_id) {
			_id = to_span_id(typeinfo<T>::id());
		}
		return _id;
	}
};
template <typename T>
class typeinfo<span<const T> >
{
protected:
	typeinfo();
public:
	inline static int id()
	{
		return typeinfo<span<T> >::id();
	}
};

template <typename T>
class typeinfo<reference<T> >
{
protected:
	typeinfo();
public:
	static int id()
	{
		return to_reference_id(typeinfo<T>::id());
	}
};
template <> inline __MPT_CONST_TYPE int typeinfo<reference <metatype> >::id()
{
	return TypeMetaRef;
}

template <typename T>
class typeinfo<item<T> >
{
protected:
	typeinfo();
public:
	static int id()
	{
		return to_item_id(typeinfo<T>::id());
	}
};

/* floating point values */
template<> inline __MPT_CONST_TYPE int typeinfo<float>::id()       { return 'f'; }
template<> inline __MPT_CONST_TYPE int typeinfo<double>::id()      { return 'd'; }
template<> inline __MPT_CONST_TYPE int typeinfo<long double>::id() { return 'e'; }
/* integer values */
template<> inline __MPT_CONST_TYPE int typeinfo<int8_t>::id()  { return 'b'; }
template<> inline __MPT_CONST_TYPE int typeinfo<int16_t>::id() { return 'n'; }
template<> inline __MPT_CONST_TYPE int typeinfo<int32_t>::id() { return 'i'; }
template<> inline __MPT_CONST_TYPE int typeinfo<int64_t>::id() { return 'x'; }
/* unsigned values */
template<> inline __MPT_CONST_TYPE int typeinfo<uint8_t>::id()  { return 'y'; }
template<> inline __MPT_CONST_TYPE int typeinfo<uint16_t>::id() { return 'q'; }
template<> inline __MPT_CONST_TYPE int typeinfo<uint32_t>::id() { return 'u'; }
template<> inline __MPT_CONST_TYPE int typeinfo<uint64_t>::id() { return 't'; }
/* string data */
template<> inline __MPT_CONST_TYPE int typeinfo<char>::id() { return 'c'; }
template<> inline __MPT_CONST_TYPE int typeinfo<char *>::id() { return 's'; }
template<> inline __MPT_CONST_TYPE int typeinfo<const char *>::id() { return 's'; }

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#ifdef __cplusplus
#endif

#endif /* _MPT_TYPES_H */
