/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_CORE_H
#define _MPT_CORE_H  @INTERFACE_VERSION@

#include <sys/types.h>
#include <stdint.h>

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

#define MPT_arrsize(a)        (sizeof(a) / sizeof(*(a)))
#define MPT_align(x)          ((x) + ((sizeof(void *))-1) - (((x)-1)&((sizeof(void *))-1)))
#define MPT_offset(s,e)       ((size_t) &(((MPT_STRUCT(s) *) 0)->e))
#define MPT_baseaddr(t,p,m)   ((MPT_STRUCT(t) *) (((int8_t *) (p)) - MPT_offset(t,m)))


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

/* calculate environment-depending hash for data */
extern uintptr_t mpt_hash(const void *, int __MPT_DEFPAR(-1));
/* calculate smdb-hash for data */
extern uintptr_t mpt_hash_smdb(const void *, int);
/* calculate djb2-hash for data */
extern uintptr_t mpt_hash_djb2(const void *, int);
/* set hash type */
extern int _mpt_hash_set(const char *);

/* next non-empty character in string */
extern int mpt_string_nextvis(const char **);

__MPT_EXTDECL_END

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
MPT_INTERFACE(convertable)
{
protected:
	inline ~convertable() { }
public:
	const char *string();
	
	inline const uint8_t *types()
	{
		uint8_t *t = 0;
		return (convert(0, &t) < 0) ? 0 : t;
	}
	inline int type()
	{
		return convert(0, 0);
	}
	
	template<typename T>
	operator T *();
	
	virtual int convert(int , void *) = 0;
	
	static const struct named_traits *pointer_traits();
};

#else
MPT_INTERFACE(convertable);
MPT_INTERFACE_VPTR(convertable)
{
	int (*convert)(MPT_INTERFACE(convertable) *, int , void *);
}; MPT_INTERFACE(convertable) {
	const MPT_INTERFACE_VPTR(convertable) *_vptr;
};
#endif

#ifdef __cplusplus
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
#endif

/* text identifier for entity */

/* value output format */
#ifdef __cplusplus
MPT_STRUCT(identifier)
{
public:
	identifier(size_t = sizeof(identifier));
	identifier(const identifier &);
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
# define MPT_CHARSET(x)  x
#else
# define MPT_CHARSET(x)  MPT_ENUM(Charset##x)
#endif

enum MPT_CHARSET(Types) {
	/* character codepoints */
	MPT_CHARSET(UTF8)       = 0x1,    /* 1-byte UTF encoding */
	MPT_CHARSET(UTF16)      = 0x2,    /* 2-byte UTF encoding */
	MPT_CHARSET(UTF32)      = 0x3,    /* 4-byte UTF encoding */
	
	/* arbitrary length hash types */
	MPT_CHARSET(HASH4)      = 0x4,    /*  4byte type-ID plus value */
	MPT_CHARSET(HASH8)      = 0x5,    /* 12byte type-ID plus value */
	/* arbitrary length identifier types */
	MPT_CHARSET(ID4)        = 0x6,    /*  4byte type-ID plus value */
	MPT_CHARSET(ID8)        = 0x7,    /* 12byte type-ID plus value */
	
	MPT_CHARSET(FixedSize)  = 0x100,
	MPT_CHARSET(ASCII)      = MPT_CHARSET(UTF8)  | MPT_CHARSET(FixedSize),
	MPT_CHARSET(UCS2)       = MPT_CHARSET(UTF16) | MPT_CHARSET(FixedSize),
	
	MPT_CHARSET(Extended)   = 0xf0    /* non-builtin character set */
};
#ifdef __cplusplus
protected:
#else
MPT_STRUCT(identifier)
{
# define MPT_IDENTIFIER_INIT   { 0, 0, 0, { 0 }, 0 }
# define MPT_IDENTIFIER_HSIZE  4
#endif
	uint16_t _len;
	uint8_t  _charset;
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

/* get convertable text/raw data */
extern const char *mpt_convertable_data(MPT_INTERFACE(convertable) *, size_t *__MPT_DEFPAR(0));


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


/* write error message and abort program */
extern void _mpt_abort(const char *, const char *, const char *, int) __attribute__ ((__noreturn__));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CORE_H */
