/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_CORE_H
#define _MPT_CORE_H  @INTERFACE_VERSION@

#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>

/* localisation makro */
#ifndef MPT_tr
# define MPT_tr(s) (s)
#endif

#ifndef MPT_ABORT
# define MPT_ABORT(t) _mpt_abort(t, __func__, __FILE__, __LINE__)
#endif

#ifdef __cplusplus

# include <ostream>

# if __cplusplus >= 201103L
#  define __MPT_CONST_EXPR constexpr
#  define __MPT_OVERRIDE override
# else
#  define __MPT_CONST_EXPR
#  define __MPT_OVERRIDE
# endif

# if defined(__clang__)
#  define __MPT_CONST_TYPE
# else
#  define __MPT_CONST_TYPE __MPT_CONST_EXPR
# endif

# define __MPT_NAMESPACE_BEGIN namespace mpt {
# define __MPT_NAMESPACE_END   }
# define __MPT_EXTDECL_BEGIN   extern "C" {
# define __MPT_EXTDECL_END     }

# define __MPT_DEFPAR(v) = (v)

# define MPT_INTERFACE(i) class i
# define MPT_STRUCT(s)    struct s
# define MPT_ENUM(e)      e
# define MPT_TYPE(t)      t##_t
# define MPT_ERROR(t)     t
#else
# define MPT_INTERFACE_VPTR(v) struct _mpt_vptr_##v
# define MPT_INTERFACE(i)      struct mpt_##i
# define MPT_STRUCT(s)         struct mpt_##s
# define MPT_ENUM(e)           MPT_##e
# define MPT_TYPE(t)           mpt_##t##_t
# define MPT_ERROR(t)          MPT_ERROR_##t

# define __MPT_DEFPAR(v)

# define __MPT_NAMESPACE_BEGIN
# define __MPT_NAMESPACE_END
# define __MPT_EXTDECL_BEGIN
# define __MPT_EXTDECL_END
#endif

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(logger);

MPT_INTERFACE(metatype);
MPT_INTERFACE(iterator);

#define MPT_arrsize(a)        (sizeof(a) / sizeof(*(a)))
#define MPT_align(x)          ((x) + ((sizeof(void *))-1) - (((x)-1)&((sizeof(void *))-1)))
#define MPT_offset(s,e)       ((size_t) &(((MPT_STRUCT(s) *) 0)->e))
#define MPT_baseaddr(t,p,m)   ((MPT_STRUCT(t) *) (((int8_t *) (p)) - MPT_offset(t,m)))

enum MPT_ENUM(Types)
{
	/* system types */
	MPT_ENUM(TypeSocket)        = 0x1,   /* SOH */
	/* system pointer types */
	MPT_ENUM(TypeFile)          = 0x4,   /* EOT */
	MPT_ENUM(TypeAddress)       = 0x5,   /* ENQ */
	
	/* format types (scalar) */
	MPT_ENUM(TypeValFmt)        = 0x8,   /* BS '\b' */
	MPT_ENUM(TypeValue)         = 0x9,   /* HT '\t' */
	MPT_ENUM(TypeProperty)      = 0xa,   /* LF '\n' */
	MPT_ENUM(TypeCommand)       = 0xb,   /* FF '\v' */
	
	/* special pointer types */
	MPT_ENUM(TypeNode)          = 0xc,   /* FF '\f' */
	MPT_ENUM(TypeReplyData)     = 0xd,   /* CR '\r' */
	
	/* reserve range for layout types */
#define MPT_type_isLayout(v)       ((v) >= 0x10 && (v) <= 0x1f)
	
	/* reserve ranges for special/format types */
#define MPT_type_isSpecial(v)     (((v) >= 0x20 && (v) <= 0x3f) \
                                || ((v) >= 0x5b && (v) <= 0x5f) \
                                || ((v) >= 0x7b && (v) <= 0x7f))
	/* range for generic base types */
	MPT_ENUM(_TypeVectorBase)    = 0x40,
	MPT_ENUM(_TypeVectorMax)     = 0x5a,
#define MPT_type_isVector(v)      ((v) >= MPT_ENUM(_TypeVectorBase) && (v) < MPT_ENUM(_TypeVectorMax))
	MPT_ENUM(TypeVector)         = '@',  /* 0x40: generic data */
#define MPT_type_vector(v)        (MPT_type_isScalar(v) || MPT_type_isExtended(v) \
                                 ? (v) - MPT_ENUM(_TypeScalarBase) + MPT_ENUM(_TypeVectorBase) \
                                 : 0)
	MPT_ENUM(_TypeScalarBase)    = 0x60,
	MPT_ENUM(_TypeScalarMax)     = 0x7a,
	MPT_ENUM(_TypeScalarSize)    = MPT_ENUM(_TypeScalarBase) - MPT_ENUM(_TypeVectorBase),
#define MPT_type_isScalar(v)      ((v) >= MPT_ENUM(_TypeScalarBase) && (v) <= MPT_ENUM(_TypeScalarMax))
#define MPT_type_isExtended(v)    ((v) >= (MPT_ENUM(_TypeDynamic) + MPT_ENUM(_TypeScalarBase)) && (v) < 0x100)
	
	/* scalar types ('a'..'z') */
	MPT_ENUM(TypeArray)          = 'a',   /* array content */
	MPT_ENUM(TypeMetaRef)        = 'm',   /* generic metatype reference */
#define MPT_type_fromVector(v)    (((v) == MPT_ENUM(_TypeVectorBase)) \
                                 ? 0 \
                                 : (MPT_type_isVector(v) || MPT_type_isExtended((v) - MPT_ENUM(_TypeVectorBase) + MPT_ENUM(_TypeScalarBase)) \
                                  ? (v) - MPT_ENUM(_TypeVectorBase) + MPT_ENUM(_TypeScalarBase) \
                                  : MPT_ERROR(BadType)))
	
#define MPT_type_isBasic(v)  (MPT_type_isScalar(v) \
                           || MPT_type_isVector(v) \
                           || MPT_type_isLayout(v))
	
	/* range for type allocations */
	MPT_ENUM(_TypeDynamic)       = 0x80,
	
	/* config interface types */
	MPT_ENUM(TypeObject)         = 0x80,
	MPT_ENUM(TypeConfig)         = 0x81,
	/* collection interface types */
	MPT_ENUM(TypeIterator)       = 0x82,
	MPT_ENUM(TypeCollection)     = 0x83,
	/* output interface types */
	MPT_ENUM(TypeLogger)         = 0x84,
	MPT_ENUM(TypeReply)          = 0x85,
	MPT_ENUM(TypeOutput)         = 0x86,
	/* other interface types */
	MPT_ENUM(TypeSolver)         = 0x88,
	/* range for dynamic interfaces */
	MPT_ENUM(_TypeInterfaceBase) = MPT_ENUM(_TypeDynamic) + 0x10,
	MPT_ENUM(_TypeInterfaceMax)  = MPT_ENUM(_TypeDynamic) + 0x3f,
#define MPT_type_isInterface(v)   ((v) >= MPT_ENUM(_TypeDynamic) \
                                && (v) < MPT_ENUM(_TypeInterfaceMax))
	
	/* range for metatype and extensions */
	MPT_ENUM(_TypeMetaBase)      = 0x0100,
	MPT_ENUM(_TypeMetaMax)       = 0x01ff,
#define MPT_type_isMetatype(v)    ((v) >= MPT_ENUM(_TypeMetaBase) && (v) <= (MPT_ENUM(_TypeMetaMax)))
	/* range for generic type extensions */
	MPT_ENUM(_TypeGenericBase)   = 0x0200,
	MPT_ENUM(_TypeGenericMax)    = 0x0fff,
	
	/* automatic range types */
	MPT_ENUM(_TypePointerBase)   = 0x1000,
#define MPT_type_pointer(v)       (((v) > 0 && (v) <= (int) MPT_ENUM(_TypeGenericMax)) ? (v) + MPT_ENUM(_TypePointerBase) : MPT_ERROR(BadType))
	MPT_ENUM(_TypeReferenceBase) = 0x2000,
#define MPT_type_reference(v)     (((v) > 0 && (v) <= (int) MPT_ENUM(_TypeGenericMax)) ? (v) + MPT_ENUM(_TypeReferenceBase) : MPT_ERROR(BadType))
	MPT_ENUM(_TypeItemBase)      = 0x3000,
#define MPT_type_item(v)          (((v) > 0 && (v) <= (int) MPT_ENUM(_TypeGenericMax)) ? (v) + MPT_ENUM(_TypeItemBase) : MPT_ERROR(BadType))
	
	/* combined regular types */
	MPT_ENUM(TypeMetaPtr)        = MPT_ENUM(_TypeMetaBase) + MPT_ENUM(_TypePointerBase),
#define MPT_type_isMetaPtr(v)     (((v) >= MPT_ENUM(TypeMetaPtr) && (v) <= (MPT_ENUM(_TypeMetaMax) + MPT_ENUM(_TypePointerBase))))
#define MPT_type_isMetaRef(v)     ((v) >= (MPT_ENUM(_TypeMetaBase) + MPT_ENUM(_TypeReferenceBase)) \
                                && (v) <= (MPT_ENUM(_TypeMetaMax) + MPT_ENUM(_TypeReferenceBase)))
	
	MPT_ENUM(_TypeSpanBase)      = 0x4000,
	
	MPT_ENUM(_TypeUserMin)       = 0x10000
};

/* tree and list operation flags */
enum MPT_ENUM(TraverseFlags) {
	/* node traverse operations */
	MPT_ENUM(TraverseLeafs)       = 0x00000001,
	MPT_ENUM(TraverseNonLeafs)    = 0x00000002,
	MPT_ENUM(TraverseAll)         = 0x00000003,
	/* node traverse order */
	MPT_ENUM(TraversePostOrder)   = 0x00000000,
	MPT_ENUM(TraversePreOrder)    = 0x00000004,
	MPT_ENUM(TraverseInOrder)     = 0x00000008,
	MPT_ENUM(TraverseLevelOrder)  = 0x0000000C,
	MPT_ENUM(TraverseOrders)      = 0x0000000C,
	MPT_ENUM(TraverseModes)       = 0x0000000f,
	/* property traverse flags */
	MPT_ENUM(TraverseChange)      = 0x00000010,
	MPT_ENUM(TraverseDefault)     = 0x00000020,
	/* alert on empty/unknown properties */
	MPT_ENUM(TraverseEmpty)       = 0x00000040,
	MPT_ENUM(TraverseUnknown)     = 0x00000080,
	/* control traverse operation */
	MPT_ENUM(TraverseStop)        = 0x00000100
};

enum MPT_ENUM(TypeErrors) {
	MPT_ERROR(BadArgument)    = -0x1,
	MPT_ERROR(BadValue)       = -0x2,
	MPT_ERROR(BadType)        = -0x3,
	MPT_ERROR(BadOperation)   = -0x4,
	MPT_ERROR(BadEncoding)    = -0x8,
	MPT_ERROR(MissingData)    = -0x10,
	MPT_ERROR(MissingBuffer)  = -0x11
};

MPT_STRUCT(encode_state)
{
#ifdef __cplusplus
	inline encode_state() : _ctx(0), done(0), scratch(0)
	{ }
#else
# define MPT_ENCODE_INIT { 0, 0, 0 }
#endif
	uintptr_t _ctx; /* state pointer */
	size_t done;    /* data in finished format */
	size_t scratch; /* unfinished data size */
};

MPT_STRUCT(decode_state)
{
#ifdef __cplusplus
	inline decode_state() : _ctx(0), curr(0)
	{
		data.pos = 0;
		data.len = 0;
		data.msg = -1;
	}
#else
# define MPT_DECODE_INIT { 0, 0, { 0, 0, -1 } }
#endif
	uintptr_t _ctx;  /* state pointer */
	
	size_t curr;     /* input data position */
	
	struct {
		size_t  pos;
		size_t  len;
		ssize_t msg;
	} data;          /* decoded data content */  
};
typedef ssize_t (*MPT_TYPE(data_encoder))(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
typedef int (*MPT_TYPE(data_decoder))(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);

__MPT_EXTDECL_BEGIN

/* set config from environment, files and arguments */
extern int mpt_init(int , char * const []);

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

/* calculate environment-depending hash for data */
extern uintptr_t mpt_hash(const void *, int __MPT_DEFPAR(-1));
/* calculate smdb-hash for data */
extern uintptr_t mpt_hash_smdb(const void *, int);
/* calculate djb2-hash for data */
extern uintptr_t mpt_hash_djb2(const void *, int);
/* set hash type */
extern int _mpt_hash_set(const char *);

/* type alias for symbol description */
extern int mpt_alias_typeid(const char *, const char **__MPT_DEFPAR(0));

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
class typeinfo
{
protected:
	typeinfo();
public:
	static int id();
};
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

/*! reduced slice with type but no data reference */
template <typename T>
class span
{
public:
	typedef T* iterator;
	
	inline span(T *a, long len) : _base(len < 0 ? 0 : a), _len(len * sizeof(T))
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
#endif

/*! generic data type and offset */
MPT_STRUCT(value)
{
#ifdef __cplusplus
	enum { Type = TypeValue };
	
	inline value(const char *v = 0) : fmt(0), ptr(v)
	{ }
	
	struct format
	{
	public:
		inline format()
		{
			set(0);
		}
		bool set(int);
		
		inline bool valid() const
		{
			return _fmt[0] != 0;
		}
		inline operator const uint8_t *() const
		{
			return _fmt;
		}
	protected:
		uint8_t _fmt[8];
	};
	template <typename T>
	bool consume(T &val)
	{
		int type;
		
		if ((type = typeinfo<T>::id()) <= 0) {
			return false;
		}
		if (fmt) {
			const T *tmp;
			if (type != *fmt
			    || !(tmp = static_cast<const T *>(ptr))) {
				return false;
			}
			val = *tmp;
			++fmt;
			ptr = tmp + 1;
			return true;
		}
		if (type == 's') {
			val = static_cast<const char *>(ptr);
			ptr = 0;
			return true;
		}
		return BadType;
	}
	
	bool set(const uint8_t *, const void *);
	value &operator =(const char *);
	
	const char *string() const;
	const void *scalar(int) const;
	void *pointer(int = 0) const;
	const struct iovec *vector(int = 0) const;
	const struct array *array(int = 0) const;
	class iterator *iterator() const;
	bool next();
#else
# define MPT_VALUE_INIT { 0, 0 }
#endif
	const uint8_t *fmt;  /* data format */
	const void    *ptr;  /* formated data */
};
#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int typeinfo<value>::id()
{
	return value::Type;
}
#endif

/*! wrapper for reference count */
MPT_STRUCT(refcount)
{
#ifdef __cplusplus
	inline refcount(uintptr_t ref = 1) : _val(ref)
	{ }
	uintptr_t raise();
	uintptr_t lower();
	
	inline uintptr_t value() const
	{
		return _val;
	}
protected:
#endif
	uintptr_t _val;
};

/* basic unref interface */
#ifdef __cplusplus
MPT_INTERFACE(instance)
{
protected:
	inline ~instance() { }
public:
	virtual void unref() = 0;
	virtual uintptr_t addref();
};
#else
MPT_INTERFACE(instance);
MPT_INTERFACE_VPTR(instance)
{
	void (*unref)(MPT_INTERFACE(instance) *);
	uintptr_t (*addref)(MPT_INTERFACE(instance) *);
}; MPT_INTERFACE(instance) {
	const MPT_INTERFACE_VPTR(instance) *_vptr;
};
#endif

#ifdef __cplusplus
inline uintptr_t instance::addref()
{
	return 0;
}
extern int convert(const void **, int , void *, int);

/*! container for reference type pointer */
template<typename T>
class reference
{
public:
	class type : public T
	{
	public:
		type(uintptr_t initial = 1) : _ref(initial)
		{ }
		void unref()
		{
			if (_ref.lower()) {
				return;
			}
			delete this;
		}
		uintptr_t addref()
		{
			return _ref.raise();
		}
	protected:
		refcount _ref;
	};
	inline reference(T *ref = 0) : _ref(ref)
	{ }
	inline reference(const reference &ref) : _ref(0)
	{
		*this = ref;
	}
	inline ~reference()
	{
		if (_ref) _ref->unref();
	}
	
	inline T *instance() const
	{ 
		return _ref;
	}
	inline void set_instance(T *ref)
	{
		if (_ref) _ref->unref();
		_ref = ref;
	}
	inline reference & operator= (reference const &ref)
	{
		T *r = ref._ref;
		if (r == _ref) {
			return *this;
		}
		if (r && !r->addref()) {
			r = 0;
		}
		if (_ref) _ref->unref();
		_ref = r;
		return *this;
	}
#if __cplusplus >= 201103L
	inline reference & operator= (reference &&ref)
	{
		T *r = ref._ref;
		ref._ref = 0;
		set_instance(r);
		return *this;
	}
#endif
	inline T *detach()
	{
		T *ref = _ref;
		_ref = 0;
		return ref;
	}
protected:
	T *_ref;
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
#endif

/* text identifier for entity */
MPT_STRUCT(identifier)
{
#ifdef __cplusplus
	identifier(size_t = sizeof(identifier));
	inline ~identifier()
	{
		set_name(0, 0);
	}
	bool equal(const char *, int) const;
	
	const char *name() const;
	bool set_name(const char *, int = -1);
	
	identifier &operator =(const identifier &);
	
	static inline __MPT_CONST_EXPR size_t minimalLength()
	{
		return 4 + sizeof(char *);
	}
	inline size_t totalSize() const
	{
		return 4 + _max;
	}
protected:
#else
# define MPT_IDENTIFIER_INIT   { 0, 0, 0, { 0 }, 0 }
# define MPT_IDENTIFIER_HSIZE  4
#endif
	uint16_t _len;
	uint8_t  _type;
	uint8_t  _max;
	char     _val[4];
	char    *_base;
};

#ifdef __cplusplus
template<typename T>
class item : public reference<T>, public identifier
{
public:
	item(T *ref = 0) : reference<T>(ref), identifier(sizeof(identifier) + sizeof(_post))
	{ }
	inline item &operator =(const identifier &id)
	{
		identifier::operator =(id);
		return *this;
	}
protected:
	char _post[32 - sizeof(identifier) - sizeof(reference<T>)];
};
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

/* auto-create wrapped reference */
template <typename T>
class container : protected reference<T>
{
public:
	inline container(T *ref = 0) : reference<T>(ref)
	{ }
	inline container(const reference<T> &ref) : reference<T>(ref)
	{ }
	virtual ~container()
	{ }
	virtual const reference<T> &ref()
	{
		if (!reference<T>::_ref) {
			reference<T>::_ref = new typename reference<T>::type;
		}
		return *this;
	}
	inline T *instance() const
	{
		return reference<T>::instance();
	}
};

/*! interface to search objects in tree */
class relation
{
public:
	inline relation(const relation *p = 0) : _parent(p)
	{ }
	virtual metatype *find(int , const char *, int = -1) const;
protected:
	virtual ~relation() {}
	const relation *_parent;
};
inline metatype *relation::find(int type, const char *name, int nlen) const
{
	return _parent ? _parent->find(type, name, nlen) : 0;
}
#endif /* C++ */

MPT_STRUCT(fdmode)
{
	int8_t   family; /* socket family */
	uint8_t  lsep;   /* line separator */
	union {
	struct {
	uint16_t open;   /* file open flags */
	uint16_t perm;   /* basic permission settings */
	} file;
	struct {
	uint8_t  type;   /* socket type */
	uint8_t  proto;  /* socket protocol */
	uint16_t port;   /* port number */
	} sock;
	} param;
	uint16_t stream; /* stream specific flags */
};

__MPT_EXTDECL_BEGIN

/* reference counter operations */
extern uintptr_t mpt_refcount_raise(MPT_STRUCT(refcount) *);
extern uintptr_t mpt_refcount_lower(MPT_STRUCT(refcount) *);


/* get file/socket properties from string */
extern int mpt_mode_parse(MPT_STRUCT(fdmode) *, const char *);


/* identifier operations */
extern MPT_STRUCT(identifier) *mpt_identifier_new(size_t);
extern const void *mpt_identifier_data(const MPT_STRUCT(identifier) *);
extern int mpt_identifier_compare(const MPT_STRUCT(identifier) *, const char *, int);
extern int mpt_identifier_inequal(const MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);
extern void mpt_identifier_init(MPT_STRUCT(identifier) *, size_t);
extern void *mpt_identifier_set(MPT_STRUCT(identifier) *, const char *, int);
extern void *mpt_identifier_copy(MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);


/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);
/* read from value */
extern int mpt_value_read(MPT_STRUCT(value) *, const char *, void *);


/* write error message and abort program */
extern void _mpt_abort(const char *, const char *, const char *, int) __attribute__ ((__noreturn__));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#ifdef __cplusplus
std::ostream &operator<<(std::ostream &, const mpt::value &);

template <typename T>
std::ostream &operator<<(std::ostream &o, mpt::span<T> d)
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
template <> std::ostream &operator<<(std::ostream &, mpt::span<char>);
template <> std::ostream &operator<<(std::ostream &, mpt::span<const char>);
#endif

#endif /* _MPT_CORE_H */
