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

# define __MPT_NAMESPACE_BEGIN namespace mpt {
# define __MPT_NAMESPACE_END   }
# define __MPT_EXTDECL_BEGIN   extern "C" {
# define __MPT_EXTDECL_END     }

# define __MPT_DEFPAR(v) = (v)

# define MPT_INTERFACE(i) class i
# define MPT_STRUCT(s)    struct s
# define MPT_ENUM(e)      e
# define MPT_TYPE(t)      t
# define MPT_ERROR(t)     t
#else
# define MPT_INTERFACE_VPTR(v) struct _mpt_vptr_##v
# define MPT_INTERFACE(i)      struct mpt_##i
# define MPT_STRUCT(s)         struct mpt_##s
# define MPT_ENUM(e)           MPT_##e
# define MPT_TYPE(t)           Mpt##t
# define MPT_ERROR(t)          MPT_ERROR_##t

# define __MPT_DEFPAR(v)

# define __MPT_NAMESPACE_BEGIN
# define __MPT_NAMESPACE_END
# define __MPT_EXTDECL_BEGIN
# define __MPT_EXTDECL_END
#endif

struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(metatype);
MPT_INTERFACE(iterator);

#define MPT_arrsize(a)        (sizeof(a) / sizeof(*(a)))
#define MPT_align(x)          ((x) + ((sizeof(void *))-1) - (((x)-1)&((sizeof(void *))-1)))
#define MPT_offset(s,e)       ((size_t) &(((MPT_STRUCT(s) *) 0)->e))
#define MPT_baseaddr(t,p,m)   ((MPT_STRUCT(t) *) (((int8_t *) (p)) - MPT_offset(t,m)))

enum MPT_ENUM(Types)
{
	/* system types */
	MPT_ENUM(TypeSocket)    = 0x1,   /* SOH */
	/* system pointer types */
	MPT_ENUM(TypeFile)      = 0x4,   /* EOT */
	MPT_ENUM(TypeAddress)   = 0x5,   /* ENQ */
	
	/* format types (scalar) */
	MPT_ENUM(TypeValFmt)    = 0x8,   /* BS '\b' */
	MPT_ENUM(TypeValue)     = 0x9,   /* HT '\t' */
	MPT_ENUM(TypeProperty)  = 0xa,   /* LF '\n' */
	
	/* special pointer types */
	MPT_ENUM(TypeCommand)   = 0xc,   /* FF '\f' */
	MPT_ENUM(TypeReplyData) = 0xd,   /* CR */
	MPT_ENUM(TypeNode)      = 0xe,   /* SO */
	
	/* reference types */
	MPT_ENUM(TypeConfig)    = 0x10,  /* DLE */
	MPT_ENUM(TypeLogger)    = 0x13,  /* DC3 */
	/* typed content data */
	MPT_ENUM(TypeReply)     = 0x14,  /* DC4 */
	MPT_ENUM(TypeIterator)  = 0x15,  /* NAK */
	MPT_ENUM(TypeRawData)   = 0x17,  /* ETB */
#define MPT_value_isInterface(v) ((v) >= MPT_ENUM(TypeConfig) \
                               && (v) < MPT_ENUM(TypeSpecial))
	/* object types */
	MPT_ENUM(TypeObject)    = 0x18,  /* CAN */
	MPT_ENUM(TypeSolver)    = 0x19,  /* EM */
	MPT_ENUM(TypeGroup)     = 0x1a,  /* SUB */
	MPT_ENUM(TypeOutput)    = 0x1b,  /* ESC */
#define MPT_value_isObject(v)  ((v) >= MPT_ENUM(TypeObject) \
                             && (v) < MPT_ENUM(TypeSpecial))
	
	/* special/format types (0x20..0x2f) */
	MPT_ENUM(TypeSpecial)   = ' ',   /* SPACE */
	
	/* reserved range for layout types (0x30..0x39) */
#define MPT_value_isLayout(v)  ((v) >= 0x30 && (v) <= 0x39)
	
	/* array types ('@'..'Z') */
	MPT_ENUM(TypeVector)    = '@',   /* 0x40: generic data */
#define MPT_value_isBasic(v)  (MPT_value_isScalar(v) || MPT_value_isVector(v) \
                            || MPT_value_isLayout(v))
	
	/* generic type on non-array position */
	MPT_ENUM(TypeMeta)      = '`',   /* 0x60: generic type */
#define MPT_value_isMetatype(v)  ((v) == MPT_ENUM(TypeMeta) \
                                || ((v) >= MPT_ENUM(_TypeMetaBase) \
                                    && (v) <= (MPT_ENUM(_TypeMetaBase)) + MPT_ENUM(_TypeDynamicMax)))
	/* scalar types ('a'..'z'..0x7f) */
	MPT_ENUM(TypeArray)     = 'a',   /* array content */
	
	/* type range checks */
#define MPT_value_isVector(v)   (((v) & ~MPT_ENUM(_TypeBaseDynamic)) >= MPT_ENUM(TypeVector) \
                              && ((v) & ~MPT_ENUM(_TypeBaseDynamic)) <  MPT_ENUM(TypeMeta))
#define MPT_value_isScalar(v)   (((v) & ~MPT_ENUM(_TypeBaseDynamic)) >= MPT_ENUM(TypeMeta) \
                              && (v) < MPT_ENUM(_TypeDynamicMax))

#define MPT_value_toVector(v)   (MPT_value_isScalar(v) \
                               ? (v) - MPT_ENUM(TypeMeta) + MPT_ENUM(TypeVector) \
                               : 0)
#define MPT_value_fromVector(v) (MPT_value_isVector(v) \
                               ? (v) - MPT_ENUM(TypeVector) + MPT_ENUM(TypeMeta) \
                               : -1)
	
	/* range for type allocations */
	MPT_ENUM(_TypeBaseDynamic)   = 0x80,
	MPT_ENUM(_TypeDynamicMax)    = 0xff,
	
	/* range for dynamic interfaces */
	MPT_ENUM(_TypeInterfaceBase) = MPT_ENUM(_TypeBaseDynamic),
	MPT_ENUM(_TypeInterfaceMax)  = MPT_ENUM(_TypeInterfaceBase) + 0x3f,
	
	/* range for dynamic base types */
	MPT_ENUM(_TypeScalarBase)    = MPT_ENUM(_TypeBaseDynamic) + MPT_ENUM(TypeMeta),
	MPT_ENUM(_TypeScalarMax)     = MPT_ENUM(_TypeScalarBase) + 0x1f,
	
	/* range for explicit metatypes */
	MPT_ENUM(_TypeMetaBase)      = 0x0100,
	MPT_ENUM(_TypeMetaMax)       = 0x01ff,
	
	/* range for generic types */
	MPT_ENUM(_TypeGenericBase)   = 0x0200,
	MPT_ENUM(_TypeGenericMax)    = 0x0fff,
	
	/* automatic range types */
	MPT_ENUM(_TypePointerBase)   = 0x1000,
	MPT_ENUM(_TypeReferenceBase) = 0x2000,
	MPT_ENUM(_TypeItemBase)      = 0x3000,
	
	/* range for map element types */
	MPT_ENUM(_TypeMapBase)       = 0x4000,
	MPT_ENUM(_TypeMapMax)        = 0x4fff,
	
	MPT_ENUM(_TypeUserMin)       = 0x10000
};

/* tree and list operation flags */
enum MPT_ENUM(TraverseFlags) {
	/* node traverse operations */
	MPT_ENUM(TraverseLeafs)       = 0x00000001,
	MPT_ENUM(TraverseNonLeafs)    = 0x00000002,
	MPT_ENUM(TraverseAll)         = 0x00000003,
	MPT_ENUM(TraverseFlags)       = 0x00000003,
	/* node traverse order */
	MPT_ENUM(TraversePostOrder)   = 0x00000000,
	MPT_ENUM(TraversePreOrder)    = 0x00000004,
	MPT_ENUM(TraverseInOrder)     = 0x00000008,
	MPT_ENUM(TraverseLevelOrder)  = 0x0000000C,
	MPT_ENUM(TraverseOrders)      = 0x0000000C,
	/* property traverse flags */
	MPT_ENUM(TraverseChange)      = 0x00000010,
	MPT_ENUM(TraverseDefault)     = 0x00000020,
	/* alert on empty/unknown properties */
	MPT_ENUM(TraverseEmpty)       = 0x00000040,
	MPT_ENUM(TraverseUnknown)     = 0x00000080
};

enum MPT_ENUM(TypeErrors) {
	MPT_ERROR(BadArgument)    = -0x1,
	MPT_ERROR(BadValue)       = -0x2,
	MPT_ERROR(BadType)        = -0x3,
	MPT_ERROR(BadOperation)   = -0x4,
	MPT_ERROR(BadEncoding)    = -0x8,
	MPT_ERROR(MissingData)    = -0x10,
	MPT_ERROR(MissingBuffer)  = -0x11,
	
	MPT_ERROR(MessageInput)      = -0x20,
	MPT_ERROR(MessageInProgress) = -0x21
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
	inline decode_state() : _ctx(0), done(0), scratch(0)
	{ }
#else
# define MPT_DECODE_INIT { 0, 0, 0 }
#endif
	uintptr_t _ctx; /* state pointer */
	size_t done;    /* processed data size */
	size_t scratch; /* working area */
};
typedef ssize_t (*MPT_TYPE(DataEncoder))(MPT_STRUCT(encode_state) *, const struct iovec *, const struct iovec *);
typedef ssize_t (*MPT_TYPE(DataDecoder))(MPT_STRUCT(decode_state) *, const struct iovec *, size_t);


__MPT_EXTDECL_BEGIN

/* get type position from data description */
extern int mpt_position(const char *, int);
/* get position offset from data description */
extern int mpt_offset(const char *, int);
/* get size for registered types */
extern ssize_t mpt_valsize(int);

/* add user scalar or pointer type */
extern int mpt_valtype_add(size_t);

/* add/check registered reference type */
extern const char *mpt_valtype_name(int);
extern int mpt_valtype_id(const char *, int);
/* add/check registered object type */
extern int mpt_valtype_meta_new(const char *);
extern int mpt_valtype_interface_new(const char *);

/* determine message ANSI color code */
extern const char *mpt_ansi_code(uint8_t);
extern const char *mpt_ansi_reset(void);

/* calculate environment-depending hash for data */
extern uintptr_t mpt_hash(const void *, size_t __MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
/*! reduced slice with type but no data reference */
template <typename T>
class Slice
{
public:
	typedef T* iterator;
	
	inline Slice(T *a, size_t len) : _base(len ? a : 0), _len(len*sizeof(T))
	{ }
	
	inline iterator begin() const
	{ return _base; }
	
	inline iterator end() const
	{ return _base + length(); }
	
	inline iterator nth(int i) const
	{
		if (i > (int) length()) return 0;
		if (i < 0 && (i += length()) < 0) return 0;
		return _base + i;
	}
	inline long length() const
	{ return _len / sizeof(T); }
	inline T *base() const
	{ return _base; };
	bool skip(long l)
	{
		if (l < 0 || l > length()) return false;
		if (!(_len -= l * sizeof(T))) _base = 0;
		else _base += l;
		return true;
	}
	bool trim(long l)
	{
		if (l < 0 || l > length()) return false;
		if (!(_len -= (l * sizeof(T)))) _base = 0;
		return true;
	}
protected:
	T *_base;
	size_t _len;
};
#endif

/*! generic data type and offset */
MPT_STRUCT(value)
{
#ifdef __cplusplus
	enum { Type = TypeValue };
	
	inline value(const char *v = 0) : fmt(0), ptr(v)
	{ }
	
	bool set(const char *, const void *);
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
	const char *fmt;  /* data format */
	const void *ptr;  /* formated data */
};

/*! single property information */
MPT_STRUCT(property)
{
#ifdef __cplusplus
    public:
	enum { Type = TypeProperty };
	
	inline property(const char *n = 0, const char *v = 0) : name(n), desc(0), val(v)
	{ }
	inline property(const char *n, const char *f, const void *d) : name(n), desc(0)
	{
		val.set(f, d);
	}
	inline property(size_t pos) : name(0), desc((char *) pos)
	{ }
#else
# define MPT_PROPERTY_INIT { 0, 0, MPT_VALUE_INIT }
#endif
	const char *name;      /* property name */
	const char *desc;      /* property [index->]description */
	MPT_STRUCT(value) val; /* element value */
};
typedef int (*MPT_TYPE(PropertyHandler))(void *, const MPT_STRUCT(property) *);

/*! wrapper for reference count */
MPT_STRUCT(refcount)
{
#ifdef __cplusplus
	inline refcount(uintptr_t ref = 1) : _val(ref)
	{ }
	uintptr_t raise();
	uintptr_t lower();
	inline uintptr_t value() const
	{ return _val; }
protected:
#endif
	uintptr_t _val;
};

/* basic unref interface */
#ifdef __cplusplus
MPT_INTERFACE(reference)
{
protected:
	inline ~reference() { }
public:
	virtual void unref() = 0;
	virtual uintptr_t addref();
};
#else
MPT_INTERFACE(reference);
MPT_INTERFACE_VPTR(reference)
{
	void (*unref)(MPT_INTERFACE(reference) *);
	uintptr_t (*addref)(MPT_INTERFACE(reference) *);
}; MPT_INTERFACE(reference) {
	const MPT_INTERFACE_VPTR(reference) *_vptr;
};
#endif

#ifdef __cplusplus
inline uintptr_t reference::addref()
{ return 0; }

extern int convert(const void **, int , void *, int);

extern int makeId();

template<typename T>
int typeIdentifier() {
	static int id = 0;
	if (!id && !(id = makeId())) {
		id = BadType;
	}
	return id;
}
template<typename T>
inline int typeIdentifier(const T &) { return typeIdentifier<T>(); }

/* floating point values */
template<> inline __MPT_CONST_EXPR int typeIdentifier<float>()       { return 'f'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<double>()      { return 'd'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<long double>() { return 'e'; }
/* integer values */
template<> inline __MPT_CONST_EXPR int typeIdentifier<int8_t>()  { return 'b'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<int16_t>() { return 'n'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<int32_t>() { return 'i'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<int64_t>() { return 'x'; }
/* unsigned values */
template<> inline __MPT_CONST_EXPR int typeIdentifier<uint8_t>()  { return 'y'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<uint16_t>() { return 'q'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<uint32_t>() { return 'u'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<uint64_t>() { return 't'; }
/* string data */
template<> inline __MPT_CONST_EXPR int typeIdentifier<char>() { return 'c'; }
/* builtin types */
template<> inline __MPT_CONST_EXPR int typeIdentifier<value>() { return value::Type; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<property>() { return property::Type; }

/* vector-type auto-cast for constant base types */
template<typename T>
inline __MPT_CONST_EXPR unsigned char vectorIdentifier()
{
	return MPT_value_toVector(typeIdentifier<T>());
}

/*! container for reference type pointer */
template<typename T>
class Reference
{
public:
	class instance : public T
	{
	public:
		instance(uintptr_t initial = 1) : _ref(initial)
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
	inline Reference(T *ref = 0) : _ref(ref)
	{ }
	inline Reference(const Reference &ref) : _ref(0)
	{
		*this = ref;
	}
	inline ~Reference()
	{
		if (_ref) _ref->unref();
	}
	
	inline T *pointer() const
	{ 
		return _ref;
	}
	inline void setPointer(T *ref)
	{
		if (_ref) _ref->unref();
		_ref = ref;
	}
	inline Reference & operator= (Reference const &ref)
	{
		T *r = ref._ref;
		if (r == _ref) return *this;
		if (r && !r->addref()) r = 0;
		if (_ref) _ref->unref();
		_ref = r;
		return *this;
	}
#if __cplusplus >= 201103L
	inline Reference & operator= (Reference &&ref)
	{
		T *r = ref._ref;
		ref._ref = 0;
		setPointer(r);
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

/*! interface to send data */
#ifdef __cplusplus
MPT_INTERFACE(logger)
{
protected:
	inline ~logger() {}
public:
	enum { Type = TypeLogger };
	
	int message(const char *, int , const char *, ...);
	
	static logger *defaultInstance();
	
	virtual int log(const char *, int, const char *, va_list) = 0;
# define MPT_LOG(x) x
#else
# define MPT_LOG(x) MPT_ENUM(Log##x)
#endif
enum MPT_ENUM(LogType) {
	MPT_LOG(Message)   = 0x0,   /* user (terminal) messages */
	MPT_LOG(Fatal)     = 0x1,
	MPT_LOG(Critical)  = 0x2,
	MPT_LOG(Error)     = 0x3,
	MPT_LOG(Warning)   = 0x4,
	MPT_LOG(Info)      = 0x8,
	MPT_LOG(Debug)     = 0x10,  /* debug level types */
	MPT_LOG(Debug2)    = 0x14,
	MPT_LOG(Debug3)    = 0x18,
	MPT_LOG(File)      = 0x20   /* use log target */
};
enum MPT_ENUM(LogFlags)
{
	MPT_ENUM(LogPrefix)   = 0x100, /* add type prefix */
	MPT_ENUM(LogSelect)   = 0x200, /* use ANSI colouring */
	MPT_ENUM(LogPretty)   = MPT_ENUM(LogPrefix) | MPT_ENUM(LogSelect),
	MPT_ENUM(LogANSIMore) = 0x400, /* no forced ANSI termination */
	
	MPT_ENUM(LogFunction) = 0x800  /* indicate source as function name */
};
#ifdef __cplusplus
};
#else
MPT_INTERFACE(logger);
MPT_INTERFACE_VPTR(logger) {
	int (*log)(MPT_INTERFACE(logger) *, const char *, int , const char *, va_list);
}; MPT_INTERFACE(logger) {
	const MPT_INTERFACE_VPTR(logger) *_vptr;
};
#endif

#ifdef __cplusplus
int critical(const char *, const char *, ... );
int error(const char *, const char *, ... );
int warning(const char *, const char *, ... );
int debug(const char *, const char *, ... );

int println(const char *, ... );

template <typename T>
inline __MPT_CONST_EXPR unsigned char typeIdentifier(Slice<T>)
{ return vectorIdentifier<T>(); }
template <typename T>
inline __MPT_CONST_EXPR unsigned char typeIdentifier(Slice<const T>)
{ return vectorIdentifier<T>(); }
#endif

/* text identifier for entity */
MPT_STRUCT(identifier)
{
#ifdef __cplusplus
	identifier(size_t = sizeof(identifier));
	inline ~identifier()
	{ setName(0); }
	
	bool equal(const char *, int) const;
	Slice<const char> nameData() const;
	const char *name() const;
	
	bool setName(const char *, int = -1);
	
	identifier &operator =(const identifier &);
	
	static inline __MPT_CONST_EXPR size_t minimalLength()
	{ return 4 + sizeof(char *); }
	
	inline size_t totalSize() const
	{ return 4 + _max; }
protected:
#else
# define MPT_IDENTIFIER_INIT { 0, 0, 0, { 0 }, 0 }
#endif
	uint16_t _len;
	uint8_t  _type;
	uint8_t  _max;
	char     _val[4];
	char    *_base;
};

#ifdef __cplusplus
int makeItemId(int);

template<typename T>
class Item : public Reference<T>, public identifier
{
public:
    Item(T *ref = 0) : Reference<T>(ref), identifier(sizeof(identifier) + sizeof(_post))
    { }
    static int type() {
        static int id = 0;
        if (!id) {
            id = makeItemId(typeIdentifier<T>());
        }
        return id;
    }
protected:
    char _post[32 - sizeof(identifier) - sizeof(Reference<T>)];
};

template <typename T>
inline int typeIdentifier(Item<T>)
{ return Item<T>::type(); }
template <typename T>
inline int typeIdentifier(Item<const T>)
{ return Item<const T>::type(); }

/* auto-create wrapped reference */
template <typename T>
class Container : protected Reference<T>
{
public:
    inline Container(T *ref = 0) : Reference<T>(ref)
    { }
    inline Container(const Reference<T> &ref) : Reference<T>(ref)
    { }
    virtual ~Container()
    { }
    virtual const Reference<T> &ref()
    {
        if (!Reference<T>::_ref) Reference<T>::_ref = new typename Reference<T>::instance;
        return *this;
    }
    inline T *pointer() const
    { return Reference<T>::pointer(); }
};
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

/* collection solver runtime data */
#ifdef __cplusplus
MPT_STRUCT(socket)
{
public:
# define MPT_SOCKETFLAG(x) x
#else
# define MPT_SOCKETFLAG(x) MPT_ENUM(Socket##x)
#endif
enum MPT_ENUM(SocketFlags) {
	MPT_SOCKETFLAG(Write)  = 0x1,
	MPT_SOCKETFLAG(Read)   = 0x2,
	MPT_SOCKETFLAG(RdWr)   = 0x3,
	MPT_SOCKETFLAG(Stream) = 0x4
};
#ifdef __cplusplus
	enum { Type = TypeSocket };
	
	inline socket(int fd = -1) : _id(fd)
	{ }
	~socket();
	
	inline bool active() const
	{ return _id >= 0; }
	
	bool bind(const char *, int = 2);
	bool open(const char *, const char *mode = "w");
	
	bool set(metatype &);
	bool set(const value *);
protected:
#else
MPT_STRUCT(socket)
{
# define MPT_SOCKET_INIT       { -1 }
# define MPT_socket_active(s)  ((s)->_id >= 0)
#endif
	int32_t  _id;     /* socket descriptor */
};
#ifdef __cplusplus
class Stream;
class Socket : public socket
{
public:
    Socket(socket * = 0);
    virtual ~Socket();
    
    enum { Type = socket::Type };
    
    int assign(const value *);
    
    virtual Reference<class Stream> accept();
};

/*! interface to search objects in tree */
class object;
class Relation
{
public:
    inline Relation(const Relation *p = 0) : _parent(p)
    { }
    virtual object *find(int , const char *, int = -1) const;
protected:
    virtual ~Relation() {}
    const Relation *_parent;
};
inline object *Relation::find(int type, const char *name, int nlen) const
{ return _parent ? _parent->find(type, name, nlen) : 0; }
#endif

__MPT_EXTDECL_BEGIN

/* reference counter operations */
extern uintptr_t mpt_refcount_raise(MPT_STRUCT(refcount) *);
extern uintptr_t mpt_refcount_lower(MPT_STRUCT(refcount) *);


/* get file/socket properties from string */
extern int mpt_mode_parse(MPT_STRUCT(fdmode) *, const char *);

/* socket operations */
extern int mpt_connect(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *);
extern int mpt_bind(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *, int);


/* identifier operations */
extern MPT_STRUCT(identifier) *mpt_identifier_new(size_t);
extern const void *mpt_identifier_data(const MPT_STRUCT(identifier) *);
extern int mpt_identifier_len(const MPT_STRUCT(identifier) *);
extern int mpt_identifier_compare(const MPT_STRUCT(identifier) *, const char *, int);
extern int mpt_identifier_inequal(const MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);
extern void mpt_identifier_init(MPT_STRUCT(identifier) *, size_t);
extern const void *mpt_identifier_set(MPT_STRUCT(identifier) *, const char *, int);
extern const void *mpt_identifier_copy(MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);


/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);
/* read from value */
extern int mpt_value_read(MPT_STRUCT(value) *, const char *, void *);

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);
/* process properties according to mask */
extern int mpt_generic_foreach(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(-1));


/* push log message */
extern int mpt_log(MPT_INTERFACE(logger) *, const char *, int , const char *, ... );
/* get default logger instance */
extern MPT_INTERFACE(logger) *mpt_log_default(void);

/* set default logger options */
extern int mpt_log_default_format(int);
extern int mpt_log_default_skip(int);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* start log message */
extern const char *mpt_log_intro(FILE *, int);
#endif

/* message type description */
extern const char *mpt_log_identifier(int);
/* determine message level */
int mpt_log_level(const char *);


/* write error message and abort program */
extern void _mpt_abort(const char *, const char *, const char *, int) __attribute__ ((__noreturn__));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#ifdef __cplusplus
std::ostream &operator<<(std::ostream &, const mpt::value &);
std::ostream &operator<<(std::ostream &, const mpt::property &);

template <typename T>
std::ostream &operator<<(std::ostream &o, mpt::Slice<T> d)
{
    typename mpt::Slice<T>::iterator begin = d.begin(), end = d.end();
    if (begin == end) return o;
    o << *(begin++);
    while (begin != end) o << ' ' << *(begin++);
    return o;
}
template <> std::ostream &operator<<(std::ostream &, mpt::Slice<char>);
template <> std::ostream &operator<<(std::ostream &, mpt::Slice<const char>);
#endif

#endif /* _MPT_CORE_H */
