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
	MPT_ENUM(TypeUnixSocket)     = 0x1,   /* SOH */
	/* system pointer types */
	MPT_ENUM(TypeFilePtr)        = 0x4,   /* EOT */
	MPT_ENUM(TypeAddressPtr)     = 0x5,   /* ENQ */
	
	/* special pointer types */
	MPT_ENUM(TypeReplyDataPtr)   = 0x8,   /* BS */
	MPT_ENUM(TypeNodePtr)        = 0x9,   /* HT' */
	MPT_ENUM(TypeBufferPtr)      = 0xb,   /* ENQ */
	
	/* format types (scalar) */
	MPT_ENUM(TypeValFmt)         = 0x18,  /* CAN */
	MPT_ENUM(TypeValue)          = 0x19,  /* EM  */
	MPT_ENUM(TypeProperty)       = 0x1a,  /* SUB */
	MPT_ENUM(TypeCommand)        = 0x1b,  /* ESC */
	MPT_ENUM(_TypeCoreSize)      = 0x20,
	
	/* range for generic base types */
	MPT_ENUM(_TypeVectorBase)    = 0x40,
	MPT_ENUM(_TypeVectorMax)     = 0x5a,
	MPT_ENUM(_TypeVectorSize)    = 0x20,
#define MPT_type_isVector(v)      ((v) >= MPT_ENUM(_TypeVectorBase) && (v) < MPT_ENUM(_TypeVectorMax))
#define MPT_type_toVector(v)      (MPT_type_isScalar(v) \
		? (uint8_t)((v) - MPT_ENUM(_TypeScalarBase) + MPT_ENUM(_TypeVectorBase)) \
		: 0)
	MPT_ENUM(_TypeScalarBase)    = 0x60,
	MPT_ENUM(_TypeScalarMax)     = 0x7a,
	MPT_ENUM(_TypeScalarSize)    = 0x20,
#define MPT_type_isScalar(v)      ((v) >= MPT_ENUM(_TypeScalarBase) && (v) <= MPT_ENUM(_TypeScalarMax))
#define MPT_type_toScalar(v)      (MPT_type_isVector(v) \
		? (uint8_t)((v) - MPT_ENUM(_TypeVectorBase) + MPT_ENUM(_TypeScalarBase)) \
		: 0)
	
	/* vector types ('@'..'Z') */
	MPT_ENUM(TypeVector)         = '@',   /* 0x40: generic data */
	/* scalar types ('a'..'z') */
	
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
#define MPT_type_isInterface(v)   ((v) >= MPT_ENUM(_TypeInterfaceBase) && (v) <= MPT_ENUM(_TypeInterfaceMax))
	
	MPT_ENUM(_TypeDynamicBase)   = MPT_ENUM(_TypeInterfaceMax) + 1,
	MPT_ENUM(_TypeDynamicMax)    = 0xff,
#define MPT_type_isDynamic(v)     ((v) >= MPT_ENUM(_TypeDynamicBase) && (v) <= MPT_ENUM(_TypeDynamicMax))
	
	MPT_ENUM(_TypeMetaPtrBase)   = 0x100,
	MPT_ENUM(_TypeMetaPtrMax)    = 0x7ff,
#define MPT_type_isMetaPtr(v)     ((v) >= MPT_ENUM(_TypeMetaPtrBase) && (v) <= MPT_ENUM(_TypeMetaPtrMax))
	MPT_ENUM(TypeMetaPtr)        = MPT_ENUM(_TypeMetaPtrBase),
#define MPT_type_isConvertable(v) ((v) == MPT_ENUM(TypeConvertablePtr) || (v) == MPT_ENUM(TypeMetaRef) || MPT_type_isMetaPtr(v))
	
	/* generic complex types */
	MPT_ENUM(_TypeValueBase)     = 0x800,
	/* static types with managed content */
	MPT_ENUM(TypeIdentifier)     = 0x800,
	MPT_ENUM(TypeMetaRef)        = 0x801,
	MPT_ENUM(TypeArray)          = 0x802,
	/* dynamic types with managed content */
	MPT_ENUM(_TypeValueAdd)      = 0x900,
	MPT_ENUM(_TypeValueMax)      = 0xfff
# define MPT_type_isShared(v) ((v) > 0 && ((v) <= MPT_ENUM(_TypeValueMax)))
	
	/* Higher type IDs (4096+) correspond to private/local types.
	 * 
	 * Can be any kind of valid guaranteed unique but fixed runtime value:
	 *  - a function address or its string alias (value._type = __func__)
	 *  - address of a (*static* or otherwise) *global* variable
	 *  - dlopen()/dlsym() return values
	 * 
	 * This assumption should generally ONLY be a problem on broken systems
	 * where data or code mappings in the NULL page (0..4095) are valid!
	 * 
	 * It could also lead to potential type confusions on atypical uses of
	 * `dlclose()`, especially on non-ASLR platforms.
	 * Normally, this type of reference would not exceed the lifetime of the
	 * corresponding code mapping it was created in and can be used with.
	 */
};

MPT_STRUCT(float80)
{
#ifdef __cplusplus
public:
	inline float80() {}
	inline float80(long double v) { *this = v; }
	
	float80 &operator =(long double);
	operator long double () const;
protected:
#endif
#if __BYTE_ORDER == __BIG_ENDIAN || __FLOAT_WORD_ORDER == __BIG_ENDIAN
	int16_t _prefix;
	uint8_t _mantissa[8];
#else
	uint8_t _mantissa[8];
	int16_t _prefix;
#endif
};

MPT_STRUCT(type_traits)
{
#ifdef __cplusplus
	inline type_traits(size_t _size, void (*_fini)(void *) = 0, int (*_init)(void *, const void *) = 0)
		: init(_init), fini(_fini), size(_size)
	{ }
	static int add_basic(size_t);
	static int add(const type_traits &);
	static const struct named_traits *add_interface(const char * = 0);
	static const struct named_traits *add_metatype(const char * = 0);
	
	static const struct type_traits  *get(int);
	static const struct named_traits *get(const char *, int = -1);
#else
# define MPT_TYPETRAIT_INIT(t)  { 0, 0, (t) }
#endif
	int  (* const init)(void *, const void *);
	void (* const fini)(void *);
	const size_t size;
};

MPT_STRUCT(named_traits)
{
#ifdef __cplusplus
	const type_traits &traits;
#else
	const MPT_STRUCT(type_traits) * const traits;
#endif
	const char * const name;
	const MPT_TYPE(value) type;
};

/*! generic data type and offset */
MPT_STRUCT(value)
{
#ifdef __cplusplus
	inline value() : _addr(0), _type(0)
	{ }
	value(const value &);
	
	value &operator=(const value &);
	
	template <typename T>
	value &operator=(const T &);
	
	template <typename T>
	bool get(T &) const;
	
	template<typename T>
	operator T *() const;
	
	int convert(value_t , void *) const;
	
	inline void clear()
	{
		_addr = 0;
		_type = 0;
	}
	inline bool set(int type, const void *ptr)
	{
		if (type <= 0 || type > _TypeValueMax) {
			return false;
		}
		_addr = ptr;
		_type = type;
		return true;
	}
	inline bool set(const char *id, const void *ptr)
	{
		value_t type = reinterpret_cast<value_t>(id);
		if (type <= _TypeValueMax) {
			return false;
		}
		_addr = ptr;
		_type = type;
		return true;
	}
	inline value_t type() const
	{
		return _type;
	}
	inline const void *data() const
	{
		return _addr;
	}
	
	const char *string() const;
	const struct iovec *vector(value_t = 0) const;
	const struct array *array(value_t = 0) const;
protected:
#else
# define MPT_VALUE_INIT(t, p) { (p), (t) }
# define MPT_value_set(v, t, p) ( \
	(v)->_addr = (p), \
	(v)->_type = (t))
#endif
	const void     *_addr;  /* address of value data */
	MPT_TYPE(value) _type;  /* value type identifier */
};

/*! generic iterator interface */
#ifdef __cplusplus
MPT_INTERFACE(iterator)
{
protected:
	inline ~iterator() {}
public:
	virtual const struct value *value() = 0;
	virtual int advance();
	virtual int reset();
	
	static const struct named_traits *pointer_traits();
	
	template <typename T>
	bool get(T &val)
	{
		const struct value *src = value();
		return src ? src->get(val) : false;
	}
};
inline int iterator::advance() {
	return MissingData;
}
inline int iterator::reset() {
	return BadOperation;
}
#else
MPT_INTERFACE(iterator);
MPT_INTERFACE_VPTR(iterator)
{
	const MPT_STRUCT(value) *(*value)(MPT_INTERFACE(iterator) *);
	int (*advance)(MPT_INTERFACE(iterator) *);
	int (*reset)(MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(iterator) {
	const MPT_INTERFACE_VPTR(iterator) *_vptr;
};
#endif


__MPT_EXTDECL_BEGIN

/* extended double representations */
extern void mpt_float80_decode(long , const MPT_STRUCT(float80) *, long double *);
extern void mpt_float80_encode(long , const long double *, MPT_STRUCT(float80) *);
extern int mpt_float80_compare(const MPT_STRUCT(float80) *, const MPT_STRUCT(float80) *);

/* query type mappings */
extern const MPT_STRUCT(type_traits) *mpt_type_traits(MPT_TYPE(value));

/* traits for registered named types */
extern const MPT_STRUCT(named_traits) *mpt_named_traits(const char *, int);
extern const MPT_STRUCT(named_traits) *mpt_interface_traits(MPT_TYPE(value));
extern const MPT_STRUCT(named_traits) *mpt_metatype_traits(MPT_TYPE(value));
/* register additional types */
extern const MPT_STRUCT(named_traits) *mpt_type_interface_add(const char *);
extern const MPT_STRUCT(named_traits) *mpt_type_metatype_add(const char *);
extern int mpt_type_add(const MPT_STRUCT(type_traits) *);
extern int mpt_type_basic_add(size_t);

/* type alias for symbol description */
extern int mpt_alias_typeid(const char *, const char **__MPT_DEFPAR(0));
/* type identifier for (unsigned) integer types */
extern char mpt_type_int(size_t);
extern char mpt_type_uint(size_t);

/* copy value content */
extern ssize_t mpt_value_copy(const MPT_STRUCT(value) *, void *, size_t);
/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);

#ifdef _VA_LIST
/* put va_list entries into value(s) */
extern int mpt_process_vararg(const char *, va_list, int (*)(void *, MPT_INTERFACE(iterator) *), void *);
extern int mpt_value_argv(void *, size_t , int , va_list);
#endif /* _VA_LIST */

/* get value from iterator and advance */
extern int mpt_iterator_consume(MPT_INTERFACE(iterator) *, MPT_TYPE(value) , void *);

__MPT_EXTDECL_END

#ifdef __cplusplus

inline __MPT_CONST_TYPE uint8_t basetype(MPT_TYPE(value) org) {
	return (org == 0)
		? 0
		: (org <= _TypeDynamicMax)
			? org
			: (org == TypeArray)
				? TypeBufferPtr
				: (MPT_type_isConvertable(org))
					? TypeConvertablePtr
					: 0;
}
inline uint8_t basetype(const value &val) {
	return basetype(val.type());
}

template<typename T>
class type_properties
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits traits(sizeof(T), _fini, _init);
		return &traits;
	}
	
	static int id(bool obtain = false) {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		if (!obtain) {
			return BadType;
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

/*! type properties for already defined types */
template<> inline __MPT_CONST_TYPE int type_properties<value>::id(bool) {
	return TypeValue;
}
template<> inline const MPT_STRUCT(type_traits) *type_properties<value>::traits() {
	return type_traits::get(id(true));
}

/*! type properties for pointer type (fallback) */
template<typename T>
class type_properties<T *>
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits traits(sizeof(T *));
		return &traits;
	}
	
	static int id(bool obtain = false) {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		if (!obtain) {
			return BadType;
		}
		return _valtype = type_traits::add(*traits());
	}
private:
	type_properties();
};

/* type properties for interface pointer types */
template<> inline __MPT_CONST_TYPE int type_properties<convertable *>::id(bool) {
	return TypeConvertablePtr;
}
template<> inline const MPT_STRUCT(type_traits) *type_properties<convertable *>::traits() {
	return type_traits::get(id(true));
}
template <> inline __MPT_CONST_TYPE int type_properties<iterator *>::id(bool) {
	return TypeIteratorPtr;
}
template <> inline const struct type_traits *type_properties<iterator *>::traits() {
	return type_traits::get(id(true));
}

/* definition with type property dependencies */
template <typename T>
inline value &value::operator=(const T &val)
{
	const int t = type_properties<T>::id(true);
	if (t <= 0) {
		_addr = 0;
		_type = 0;
	}
	else {
		_addr = &val;
		_type = t;
	}
	return *this;
}
template <typename T>
bool value::get(T &val) const
{
	return convert(type_properties<T>::id(true), &val) >= 0;
}

/*! vector data compatible to `struct iovec` memory */
template <typename T>
class span
{
public:
	typedef T* iterator;
	
	inline span(T *a, long len) : _base(len < 0 ? 0 : a), _len(len * sizeof(T))
	{ }
	inline span() : _base(0), _len(0)
	{ }
	
	inline iterator begin() const
	{
		return _base;
	}
	inline iterator end() const
	{
		return _base + size();
	}
	inline long size() const
	{
		return _len / sizeof(T);
	}
	inline size_t size_bytes() const
	{
		return _len;
	}
	inline iterator nth(long pos) const
	{
		if (pos < 0) {
			if ((pos += size()) < 0) {
				return 0;
			}
		}
		else if (pos >= size()) {
			return 0;
		}
		return _base + pos;
	}
	bool skip(long l)
	{
		if (l < 0 || l > size()) {
			return false;
		}
		_len -= l * sizeof(T);
		_base += l;
		return true;
	}
	bool trim(long l)
	{
		if (l < 0 || l > size()) {
			return false;
		}
		_len -= l * sizeof(T);
		return true;
	}
protected:
	T *_base;
	size_t _len;
};
/*! special properties for base type resolution */
template<typename T>
class type_properties<span<const T> >
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits *traits = 0;
		if (traits) {
			return traits;
		}
		int type = id(true);
		if (type < 0 || !(traits = type_traits::get(type))) {
			traits = _dynamic_traits();
		}
		return traits;
	}
	
	static int id(bool obtain = false) {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		/* use vector IDs for builtin types */
		int type = type_properties<T>::id(false);
		if ((type = MPT_type_toVector(type)) > 0) {
			return _valtype = type;
		}
		if (!obtain) {
			return BadType;
		}
		return _valtype = type_traits::add(*_dynamic_traits());
	}
private:
	static const type_traits *_dynamic_traits() {
		static const struct type_traits traits(sizeof(span<T>));
		return &traits;
	}
	type_properties();
};
/*! set traits to trivially copyable data */
template<typename T>
class type_properties<span<T> >
{
public:
	static const struct type_traits *traits(void) {
		static const struct type_traits traits(sizeof(span<T>));
		return &traits;
	}
	
	static int id(bool obtain = false) {
		static int _valtype = 0;
		if (_valtype > 0) {
			return _valtype;
		}
		if (!obtain) {
			return BadType;
		}
		return _valtype = type_traits::add(*traits());
	}
private:
	type_properties();
};

template<typename T>
bool assign(const T &from, int type, void *ptr) {
	/* bad arg or type ID registration error */
	if (type <= 0 || type != type_properties<T>::id(true)) {
		return false;
	}
	if (ptr) {
		*static_cast<T *>(ptr) = from;
	}
	return true;
}

/* floating point values */
template<> inline __MPT_CONST_TYPE int type_properties<float>::id(bool)       { return 'f'; }
template<> inline __MPT_CONST_TYPE int type_properties<double>::id(bool)      { return 'd'; }
template<> inline __MPT_CONST_TYPE int type_properties<long double>::id(bool) { return 'e'; }
/* integer values */
template<> inline __MPT_CONST_TYPE int type_properties<int8_t>::id(bool)  { return 'b'; }
template<> inline __MPT_CONST_TYPE int type_properties<int16_t>::id(bool) { return 'n'; }
template<> inline __MPT_CONST_TYPE int type_properties<int32_t>::id(bool) { return 'i'; }
template<> inline __MPT_CONST_TYPE int type_properties<int64_t>::id(bool) { return 'x'; }
/* unsigned values */
template<> inline __MPT_CONST_TYPE int type_properties<uint8_t>::id(bool)  { return 'y'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint16_t>::id(bool) { return 'q'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint32_t>::id(bool) { return 'u'; }
template<> inline __MPT_CONST_TYPE int type_properties<uint64_t>::id(bool) { return 't'; }
/* string data */
template<> inline __MPT_CONST_TYPE int type_properties<char>::id(bool) { return 'c'; }
template<> inline __MPT_CONST_TYPE int type_properties<const char *>::id(bool) { return 's'; }


/* floating point values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<float>::traits()       { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<double>::traits()      { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<long double>::traits() { return type_traits::get(id(true)); }
/* integer values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<int8_t>::traits()  { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int16_t>::traits() { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int32_t>::traits() { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<int64_t>::traits() { return type_traits::get(id(true)); }
/* unsigned values */
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint8_t>::traits()  { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint16_t>::traits() { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint32_t>::traits() { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<uint64_t>::traits() { return type_traits::get(id(true)); }
/* string data */
template<> inline const MPT_STRUCT(type_traits) *type_properties<char>::traits() { return type_traits::get(id(true)); }
template<> inline const MPT_STRUCT(type_traits) *type_properties<const char *>::traits() { return type_traits::get(id(true)); }

/*! generic source for values */
template <typename T>
class source : public iterator, public span<const T>
{
public:
	source(const T *val, long len = 1, int step = 1) : span<const T>(val, len), _step(step)
	{
		int type = type_properties<T>::id(true);
		_type = type < 0 ? 0 : type;
		_pos = (step < 0) ? len - 1 : 0;
	}
	virtual ~source()
	{ }
	const struct value *value() __MPT_OVERRIDE
	{
		const T *val;
		if (_pos < 0 || !(val = span<const T>::nth(_pos))) {
			return 0;
		}
		return _val.set(_type, val) ? &_val : 0;
	}
	int advance() __MPT_OVERRIDE
	{
		if (_pos < 0 || _pos >= span<const T>::size()) {
			return MissingData;
		}
		_pos += _step;
		if (_pos < 0 || _pos >= span<const T>::size()) {
			return 0;
		}
		return _type ? _type : type_properties<struct value>::id(true);
	}
	int reset() __MPT_OVERRIDE
	{
		_pos = (_step < 0) ? span<const T>::size() - 1 : 0;
		return span<const T>::size();
	}
protected:
	struct value _val;
	long _pos;
	int _step;
	int _type;
};
template <typename T>
class type_properties<source<T> *>
{
protected:
	type_properties();
public:
	static inline __MPT_CONST_EXPR int id(bool) {
		return TypeIteratorPtr;
	}
	static inline const struct type_traits *traits(void) {
		return type_traits::get(id(true));
	}
};
#endif

__MPT_NAMESPACE_END

#ifdef __cplusplus
std::ostream &operator<<(std::ostream &, const mpt::value &);

template <typename T>
std::ostream &operator<<(std::ostream &o, const mpt::span<T> &d)
{
	typename mpt::span<T>::iterator begin = d.begin(), end = d.end();
	if (begin == end) {
		return o;
	}
	o << *begin;
	while (++begin != end) {
		o << ' ' << *begin;
	}
	return o;
}
template <> std::ostream &operator<<(std::ostream &, const mpt::span<char> &);
template <> std::ostream &operator<<(std::ostream &, const mpt::span<const char> &);

template <typename T>
bool mpt::convertable::get(T &val)
{
	int type = type_properties<T>::id(true);
	return (type > 0) && (convert(type, &val) >= 0);
}
template<typename T>
mpt::convertable::operator T *()
{
	int type = mpt::type_properties<T *>::id(true);
	T *ptr = 0;
	if ((type <= 0) || (convert(type, &ptr) < 0)) {
		return 0;
	}
	return ptr;
}

template<typename T>
mpt::value::operator T *() const
{
	int type = mpt::type_properties<T *>::id(true);
	if (type < 0 || !_type || !_addr) {
		return 0;
	}
	if (_type == type) {
		return *static_cast<T * const *>(_addr);
	}
	T *ptr = 0;
	if (MPT_type_isConvertable(_type)) {
		mpt::convertable *conv = *static_cast<mpt::convertable * const *>(_addr);
		if (!conv || conv->convert(type, &ptr) < 0) {
			return 0;
		}
	}
	return ptr;
}
#endif /* __cplusplus */

#endif /* _MPT_TYPES_H */
