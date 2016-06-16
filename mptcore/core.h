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

# if __cplusplus >= 201103L
#  define __MPT_CONST_EXPR constexpr
# else
#  define __MPT_CONST_EXPR
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

MPT_STRUCT(node);

#define MPT_arrsize(a)        (sizeof(a) / sizeof(*(a)))
#define MPT_align(x)          ((x) + ((sizeof(void *))-1) - (((x)-1)&((sizeof(void *))-1)))
#define MPT_offset(s,e)       ((size_t) &(((MPT_STRUCT(s) *) 0)->e))
#define MPT_reladdr(s,b,c,t)  ((void *) (((int8_t *) (b)) + MPT_offset(s,t) - MPT_offset(s,c)))

enum MPT_ENUM(Types)
{
	/* system types (scalar) */
	MPT_ENUM(TypeSocket)    = 0x1,   /* SOH */
	MPT_ENUM(TypeValue)     = 0x2,   /* STX */
	MPT_ENUM(TypeProperty)  = 0x3,   /* ETX */
	
	/* layout types (scalar) */
	MPT_ENUM(TypeLineAttr)  = 0x4,   /* EOT */
	MPT_ENUM(TypeColor)     = 0x5,   /* ENQ rgba(0..255) */
	MPT_ENUM(TypeLine)      = 0x6,   /* ACK */
	
	/* number output format */
	MPT_ENUM(TypeValFmt)    = 0x7,   /* BEL '\a' */
	
	/* system pointer types */
	MPT_ENUM(TypeAddress)   = 0x8,   /* BS  '\b' */
	MPT_ENUM(TypeFile)      = 0x9,   /* HT  '\t' */
	MPT_ENUM(TypeNode)      = 0xa,   /* LF  '\n' */
	
	/* layout pointer types */
	MPT_ENUM(TypeText)      = 0xc,   /* FF  '\f' */
	MPT_ENUM(TypeAxis)      = 0xd,   /* CR  '\r' */
	MPT_ENUM(TypeWorld)     = 0xe,   /* SO */
	MPT_ENUM(TypeGraph)     = 0xf,   /* SI */
	
	/* reference types */
	MPT_ENUM(TypeMeta)      = 0x10,  /* CAN */
	MPT_ENUM(TypeIODevice)  = 0x11,  /* DC1 */
	MPT_ENUM(TypeInput)     = 0x12,  /* DC2 */
	MPT_ENUM(TypeLogger)    = 0x13,  /* DC3 */
	MPT_ENUM(TypeCycle)     = 0x14,  /* DC4 */
	
	/* object types */
	MPT_ENUM(TypeObject)    = 0x18,  /* CAN */
	MPT_ENUM(TypeSolver)    = 0x19,  /* EM */
	MPT_ENUM(TypeGroup)     = 0x1a,  /* SUB */
	MPT_ENUM(TypeOutput)    = 0x1b,  /* ESC */
	
	/* vector types (0x20..0x3f) */
	MPT_ENUM(TypeVecBase)   = ' ',   /* 0x20: generic vector */
	
	/* array types ('@'..'Z'..0x5f) */
	MPT_ENUM(TypeArrBase)   = '@',   /* 0x40: generic array */
	
	/* scalar types ('`'..'z'..0x7f) */
	MPT_ENUM(TypeScalBase)  = '`',   /* 0x60: generic scalar */
	
	/* reuse value for transfer-only 80bit float */
	MPT_ENUM(TypeFloat80)   = MPT_ENUM(TypeScalBase),
	
	/* types with printable representation ('a'..'z') */
#if __SIZEOF_LONG__ == 8
	MPT_ENUM(TypeLong)      = 'x',
	MPT_ENUM(TypeULong)     = 't',
#elif __SIZEOF_LONG__ == 4
	MPT_ENUM(TypeLong)      = 'i',
	MPT_ENUM(TypeULong)     = 'u',
#else
# error: bad sizeof(long)
#endif
#ifdef __SIZEOF_LONG_LONG__
 #if __SIZEOF_LONG_LONG__ == 8
	MPT_ENUM(TypeLongLong)  = 'x',
	MPT_ENUM(TypeULongLong) = 't',
# elif __SIZEOF_LONG_LONG__ == 4
	MPT_ENUM(TypeLongLong)  = 'i',
	MPT_ENUM(TypeULongLong) = 'u',
# else
#  error: bad sizeof(long)
# endif
#endif
	MPT_ENUM(TypeUser)      = 0x80,
	MPT_ENUM(_TypeFinal)    = 0xff,
	
	MPT_ENUM(ValueConsume)  = 0x100
};

enum MPT_ENUM(LogType) {
	MPT_ENUM(LogMessage)   = 0x0,   /* user (terminal) messages */
	MPT_ENUM(LogFatal)     = 0x1,
	MPT_ENUM(LogCritical)  = 0x2,
	MPT_ENUM(LogError)     = 0x10,
	MPT_ENUM(LogWarning)   = 0x20,
	MPT_ENUM(LogInfo)      = 0x30,
	MPT_ENUM(LogDebug)     = 0x40,  /* debug level types */
	MPT_ENUM(LogDebug2)    = 0x50,
	MPT_ENUM(LogDebug3)    = 0x60,
	MPT_ENUM(LogDebug4)    = 0x70,
	MPT_ENUM(LogFile)      = 0x80,  /* use log target */
	
	MPT_ENUM(LogPrefix)    = 0x100, /* add type prefix */
	MPT_ENUM(LogSelect)    = 0x200, /* use ANSI colouring */
	MPT_ENUM(LogANSIMore)  = 0x400, /* no forced ANSI termination */
	MPT_ENUM(LogPretty)    = 0x700,
	
	MPT_ENUM(LogFunction)  = 0x800  /* auto-add function decorator */
};
#define MPT_FCNLOG(x) (MPT_ENUM(Log##x) | MPT_ENUM(LogFunction))

enum MPT_ENUM(SocketFlags) {
	MPT_ENUM(SocketStream) = 0x1,
	MPT_ENUM(SocketWrite)  = 0x2,
	MPT_ENUM(SocketRead)   = 0x4
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

MPT_STRUCT(codestate)
{
#ifdef __cplusplus
	inline codestate() : _ctx(0), done(0), scratch(0)
	{ }
#else
# define MPT_CODESTATE_INIT { 0, 0, 0 }
#endif
	uintptr_t _ctx; /* state pointer */
	size_t done;    /* data in finished format */
	size_t scratch; /* unfinished data size */
};
typedef ssize_t (*MPT_TYPE(DataEncoder))(MPT_STRUCT(codestate) *, const struct iovec *, const struct iovec *);
typedef ssize_t (*MPT_TYPE(DataDecoder))(MPT_STRUCT(codestate) *, const struct iovec *, size_t);

/*! generic struct reference */
MPT_STRUCT(value)
{
#ifdef __cplusplus
	enum { Type = TypeValue };
	
	inline value(const char *f, const void *v) : fmt(f), ptr(v)
	{ }
	inline value(const char *v = 0) : fmt(0), ptr(v)
	{ }
	inline void set(const struct value &v)
	{ fmt = v.fmt; ptr = v.ptr; }
	
	static bool isPointer(int);
	static bool isScalar(int);
	static bool isVector(int);
	static bool isArray(int);
#else
# define MPT_value_isVector(v)   (((v) & 0x7f) >= MPT_ENUM(TypeVecBase) && \
                                  ((v) & 0x7f) < MPT_ENUM(TypeArrBase))
# define MPT_value_isArray(v)    (((v) & 0x7f) >= MPT_ENUM(TypeArrBase) && \
                                  ((v) & 0x7f) < MPT_ENUM(TypeScalBase))
# define MPT_value_fromVector(v) (MPT_value_isVector(v) ? \
                                  (v) - MPT_ENUM(TypeVecBase) + MPT_ENUM(TypeScalBase) : MPT_ERROR(BadValue))
# define MPT_value_fromArray(v)  (MPT_value_isArray(v) ? \
                                  (v) - MPT_ENUM(TypeArrBase) + MPT_ENUM(TypeScalBase) : MPT_ERROR(BadValue))
# define MPT_value_toVector(v)   (((v) & 0x7f) < MPT_ENUM(TypeScalBase) ? 0 : (v) - MPT_ENUM(TypeScalBase) + MPT_ENUM(TypeVecBase))
# define MPT_value_toArray(v)    (((v) & 0x7f) < MPT_ENUM(TypeScalBase) ? 0 : (v) - MPT_ENUM(TypeScalBase) + MPT_ENUM(TypeArrBase))
#endif
	const char *fmt;  /* data format */
	const void *ptr;  /* formated data */
};

/*! wrapper for references */
MPT_STRUCT(reference)
{
#ifdef __cplusplus
	inline reference(uintptr_t ref = 1) : _val(ref)
	{ }
	uintptr_t raise();
	uintptr_t lower();
protected:
#endif
	uintptr_t _val;
};

/*! single property information */
MPT_STRUCT(property)
{
#ifdef __cplusplus
    public:
	enum { Type = TypeProperty };
	
	inline property(const char *n = "", const char *v = 0) : name(n), desc(0), val(0, v)
	{ }
	inline property(const char *n, const char *f, const void *d) : name(n), desc(0), val(f, d)
	{ }
	inline property(size_t pos) : name(0), desc((char *) pos)
	{ }
	static int convertAssign(int v)
	{ return (v * 0x10000) | Type; }
#else 
# define MPT_property_assign(v) (((v) * 0x10000) | MPT_ENUM(TypeProperty))
#endif
	const char *name;      /* property name */
	const char *desc;      /* property [index->]description */
	MPT_STRUCT(value) val; /* element value */
};
typedef int (*MPT_TYPE(PropertyHandler))(void *, const MPT_STRUCT(property) *);

#ifdef __cplusplus
class Transform;

extern int convert(const void **, int , void *, int);

template<typename T>
inline __MPT_CONST_EXPR int typeIdentifier() { return T::Type; }
template<typename T>
inline __MPT_CONST_EXPR int typeIdentifier(const T &) { return typeIdentifier<T>(); }

/* floating point values */
template<> inline __MPT_CONST_EXPR int typeIdentifier<float>()  { return 'f'; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<double>() { return 'd'; }
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

#if __SIZEOF_LONG__ != 8
/* TODO: better detection when needed/conflicting */
template<> inline __MPT_CONST_EXPR int typeIdentifier<long>() { return TypeLong; }
template<> inline __MPT_CONST_EXPR int typeIdentifier<unsigned long>() { return TypeULong; }
#endif

template<> inline __MPT_CONST_EXPR int typeIdentifier<long double>()
{
	return sizeof(long double) == 16 ? 'e'
	: sizeof(long double) == 12 ? MPT_ENUM(TypeFloat80)
	: sizeof(long double) == 8 ? 'd'
	: sizeof(long double) == 4 ? 'f'
	: 0;
}

template<typename T>
inline __MPT_CONST_EXPR int vectorIdentifier() {
    return (typeIdentifier<T>() > _TypeFinal ||
            (typeIdentifier<T>() & ~TypeUser) < TypeScalBase)
              ? 0 : typeIdentifier<T>() - TypeScalBase + TypeVecBase;
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
        reference _ref;
    };
    inline Reference(T *ref = 0) : _ref(ref)
    { }
    inline Reference(const Reference &ref) : _ref(0)
    { *this = ref; }
    inline ~Reference()
    { if (_ref) _ref->unref(); }
    
    inline T *pointer() const
    { return _ref; }
    
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
    inline T *detach()
    { T *ref = _ref; _ref = 0; return ref; }
protected:
    T *_ref;
};
#endif

/*! interface to send data */
MPT_INTERFACE(logger)
#ifdef __cplusplus
{
protected:
	inline ~logger() {}
public:
	enum { Type = TypeLogger };
	
	enum LogTypes {
		Fatal    = MPT_FCNLOG(Fatal),
		Critical = MPT_FCNLOG(Critical),
		Error    = MPT_FCNLOG(Error),
		Warning  = MPT_FCNLOG(Warning),
		Info     = MPT_FCNLOG(Info),
		Debug    = MPT_FCNLOG(Debug),
		File     = MPT_FCNLOG(File)
	};
	int message(const char *, int , const char *, ...);
	
	static logger *defaultInstance();
	
	virtual void unref() = 0;
	virtual int log(const char *, int, const char *, va_list) = 0;
#else
; MPT_INTERFACE_VPTR(logger) {
	void (*unref)(MPT_INTERFACE(logger) *);
	int (*log)  (MPT_INTERFACE(logger) *, const char *, int , const char *, va_list);
}; MPT_INTERFACE(logger) {
	const MPT_INTERFACE_VPTR(logger) *_vptr;
#endif
};
int critical(const char *, const char *, ... );
int error(const char *, const char *, ... );
int warning(const char *, const char *, ... );
int debug(const char *, const char *, ... );

int message(const char *, ... );

/*! generic metatype interface */
MPT_INTERFACE(metatype)
#ifdef __cplusplus
{
protected:
	inline ~metatype() {}
public:
	enum { Type = TypeMeta };
	
	const char *string();
	
	template <typename T>
	inline T *cast()
	{
	    int t = typeIdentifier<T>();
	    if (!value::isPointer(t)) return 0;
	    T *ptr;
	    if (conv(t, &ptr) < 0) return 0;
	    return ptr;
	}
	inline operator const char *()
	{ return string(); }
	
	static metatype *create(size_t size);
	
	virtual void unref() = 0;
	virtual int assign(const value *);
	virtual int conv(int, void *);
	virtual metatype *clone();
	
	inline int type()
	{ return conv(0, 0); }
#else
; MPT_INTERFACE_VPTR(metatype)
{
	void (*unref)(MPT_INTERFACE(metatype) *);
	int (*assign)(MPT_INTERFACE(metatype) *, const MPT_INTERFACE(value) *);
	int (*conv)(MPT_INTERFACE(metatype) *, int, void *);
	MPT_INTERFACE(metatype) *(*clone)(MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(metatype) {
	const MPT_INTERFACE_VPTR(metatype) *_vptr;
#endif
};

/*! generic object interface */
MPT_INTERFACE(object)
#ifdef __cplusplus
{
protected:
	inline ~object() {}
public:
	enum { Type = TypeObject };
	
	class iterator;
	class const_iterator;
	
	class iterator begin();
	class iterator end();
	
	class const_iterator const_begin() const;
	class const_iterator const_end() const;
	class const_iterator begin() const;
	class const_iterator end() const;
	
	bool set(const char *, const value &, logger * = logger::defaultInstance());
	bool setProperties(const object &, logger * = logger::defaultInstance());
	
	inline int type() const
	{ return property(0); }
	
	virtual void unref() = 0;
	virtual uintptr_t addref();
	virtual int property(struct property *) const = 0;
	virtual int setProperty(const char *, metatype * = 0) = 0;
#else
; MPT_INTERFACE_VPTR(object) {
	void (*unref)(MPT_INTERFACE(object) *);
	uintptr_t (*addref)(MPT_INTERFACE(object) *);
	int (*property)(const MPT_INTERFACE(object) *, MPT_STRUCT(property) *);
	int (*setProperty)(MPT_INTERFACE(object) *, const char *, MPT_INTERFACE(metatype) *);
}; MPT_INTERFACE(object) {
	const MPT_INTERFACE_VPTR(object) *_vptr;
#endif
};


#ifdef __cplusplus
inline metatype *metatype::clone()
{ return 0; }

inline uintptr_t object::addref()
{ return 0; }

/* specialize metatype string cast */
template <> inline const char *metatype::cast<const char>()
{ return string(); }

/* special copy for metatype */
template <> inline Reference<metatype> & Reference<metatype>::operator= (Reference<metatype> const &ref)
{
    metatype *r = ref._ref;
    if (r) r = r->clone();
    if (_ref) _ref->unref();
    _ref = r;
    return *this;
}

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
    { return _base+length(); }
    
    inline size_t length() const
    { return _len / sizeof(T); }
    inline T *base() const
    { return _base; };
    bool skip(size_t l)
    {
        if (l > length()) return false;
        if (!(_len -= l * sizeof(T))) _base = 0;
        else _base += l;
        return true;
    }
    bool trim(size_t l)
    {
        if (l > length()) return false;
        if (!(_len -= (l * sizeof(T)))) _base = 0;
        return true;
    }
    const char *fmt()
    {
        static const char fmt[] = { vectorIdentifier<T>(), 0 };
        return fmt;
    }
protected:
    T *_base;
    size_t _len;
};


/* generic implementation for metatype */
class Metatype : public metatype
{
public:
    void unref();
    int assign(const value *);
    int conv(int, void *);
    metatype *clone();

    Slice<const char> data() const;
    class Small;
    class Big;

    static Metatype *create(size_t size);

protected:
    Metatype(size_t post = 0);
    virtual ~Metatype();

    uint64_t _info;
};

class Metatype::Small : public Metatype
{
public:
    Small() : Metatype(sizeof(data))
    { }
protected:
    friend class Metatype;
    int8_t data[64-sizeof(Metatype)];
};
class Metatype::Big : public Metatype
{
public:
    Big() : Metatype(sizeof(data))
    { }
protected:
    friend class Metatype;
    int8_t data[256-sizeof(Metatype)];
};
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
	bool setName(size_t , const void *);
	bool setName(const identifier &);
protected:
#else
# define MPT_IDENTIFIER_INIT { 0, 0, 0, { 0 }, 0 }
#endif
	uint16_t _len;
	uint8_t  _post;
	uint8_t  _flags;
	char     _val[4];
	char    *_base;
};

#ifdef __cplusplus
template<typename T>
class Item : public Reference<T>, public identifier
{
public:
    Item(T *ref = 0) : Reference<T>(ref), identifier(sizeof(identifier) + sizeof(_post))
    { }
protected:
    char _post[32 - sizeof(identifier) - sizeof(Reference<T>)];
};

/*! interface to search metatypes in tree */
class Relation
{
public:
    inline Relation(const Relation *p = 0) : _parent(p)
    { }
    virtual metatype *find(int , const char *, int = -1) const = 0;
protected:
    virtual ~Relation() {}
    const Relation *_parent;
};
/*! interface to generic groups of metatypes elements */
class Group : public object
{
public:
    enum { Type = TypeGroup };
    
    int property(struct property *) const;
    int setProperty(const char *, metatype *);
    
    virtual const Item<metatype> *item(size_t pos) const;
    virtual Item<metatype> *append(metatype *);
    virtual size_t clear(const metatype * = 0);
    virtual bool bind(const Relation &from, logger * = logger::defaultInstance());
    
    virtual const Transform &transform();
    
    bool copy(const Group &from, logger * = 0);
    bool addItems(node *head, const Relation *from = 0, logger * = logger::defaultInstance());
    
protected:
    inline ~Group() {}
    virtual metatype *create(const char *, int = -1);
};
/*! Relation implemetation using Group as current element */
class GroupRelation : public Relation
{
public:
    inline GroupRelation(const Group &g, const Relation *p = 0, char sep = ':') : Relation(p), _curr(g), _sep(sep)
    { }
    virtual ~GroupRelation()
    { }
    metatype *find(int type, const char *, int = -1) const;
protected:
    const Group &_curr;
    char _sep;
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
MPT_STRUCT(socket)
{
#ifdef __cplusplus
public:
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
# define MPT_SOCKET_INIT       { -1 }
# define MPT_socket_active(s)  ((s)->_id >= 0)
#endif
	int32_t  _id;     /* message descriptor */
};
#ifdef __cplusplus
class Stream;
class Socket : public metatype, public socket
{
public:
    Socket(socket * = 0);
    virtual ~Socket();
    
    enum { Type = socket::Type };
    
    void unref();
    int assign(const value *);
    int conv(int, void *);
    
    virtual Reference<class Stream> accept();
};
#endif

__MPT_EXTDECL_BEGIN


/* get type position from data description */
extern int mpt_position(const char *, int);
/* get position offset from data description */
extern int mpt_offset(const char *, int);
/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);

/* get/add registered (primitive) types */
extern ssize_t mpt_valsize(int);
extern int mpt_valtype_add(size_t);


/* reference operations */
extern uintptr_t mpt_reference_raise(MPT_STRUCT(reference) *);
extern uintptr_t mpt_reference_lower(MPT_STRUCT(reference) *);


/* calculate environment-depending hash for data */
extern uintptr_t mpt_hash(const void *, size_t __MPT_DEFPAR(0));

/* get file/socket properties from string */
extern int mpt_mode_parse(MPT_STRUCT(fdmode) *, const char *);

/* socket operations */
extern int mpt_connect(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *);
extern int mpt_bind(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *, int);


/* identifier operations */
size_t mpt_identifier_align(size_t);
extern const void *mpt_identifier_data(const MPT_STRUCT(identifier) *);
extern int mpt_identifier_len(const MPT_STRUCT(identifier) *);
extern int mpt_identifier_compare(const MPT_STRUCT(identifier) *, const char *, int);
extern int mpt_identifier_inequal(const MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);
extern void mpt_identifier_init(MPT_STRUCT(identifier) *, size_t);
extern const void *mpt_identifier_set(MPT_STRUCT(identifier) *, const char *, int);
extern const void *mpt_identifier_copy(MPT_STRUCT(identifier) *, const MPT_STRUCT(identifier) *);


/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(size_t);
/* new generic metatype with same data and identifier */
extern MPT_INTERFACE(metatype) *mpt_meta_clone(MPT_INTERFACE(metatype) *);

/* get node/metatype text/raw data */
extern const char *mpt_meta_data(MPT_INTERFACE(metatype) *, size_t *__MPT_DEFPAR(0));


/* get object type name */
extern const char *mpt_object_typename(MPT_INTERFACE(object) *);

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);

/* loop trough metatype/generic properties */
extern int mpt_object_foreach(const MPT_INTERFACE(object) *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));

/* set metatype property to match argument */
extern int mpt_object_pset(MPT_INTERFACE(object) *, const char *, const MPT_STRUCT(value) *, const char * __MPT_DEFPAR(0));
extern int mpt_object_vset(MPT_INTERFACE(object) *, const char *, const char *, va_list);
extern int mpt_object_set (MPT_INTERFACE(object) *, const char *, const char *, ... );


/* initialize geninfo data */
extern int _mpt_geninfo_init(void *, size_t);
/* operations on geninfo data */
extern int _mpt_geninfo_value(uint64_t *, const MPT_STRUCT(value) *);
extern int _mpt_geninfo_line(const uint64_t *);
extern int _mpt_geninfo_conv(const uint64_t *, int , void *);
/* create new metatype with data */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const uint64_t *);


/* get logging interface pointer from object data */
extern MPT_INTERFACE(logger) *mpt_object_logger(const MPT_INTERFACE(object) *);
/* log output */
extern int mpt_log(MPT_INTERFACE(logger) *, const char *, int , const char *, ... );
#if defined(_STDIO_H) || defined(_STDIO_H_)
/* get default logger instance */
extern MPT_INTERFACE(logger) *mpt_log_default(int __MPT_DEFPAR(0), FILE *__MPT_DEFPAR(0));
/* start log message */
extern const char *mpt_log_start(FILE *, const char *, int);
#endif

/* write error message and abort program */
extern void _mpt_abort(const char *, const char *, const char *, int) __attribute__ ((__noreturn__));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CORE_H */
