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

MPT_STRUCT(node);

MPT_INTERFACE(output);

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
	MPT_ENUM(TypeUnrefable) = 0x10,  /* DLE */
	MPT_ENUM(TypeIODevice)  = 0x11,  /* DC1 */
	MPT_ENUM(TypeInput)     = 0x12,  /* DC2 */
	MPT_ENUM(TypeLogger)    = 0x13,  /* DC3 */
	MPT_ENUM(TypeMeta)      = 0x14,  /* DC4 */
	MPT_ENUM(TypeRawData)   = 0x15,  /* NAK */
#define MPT_value_isUnrefable(v) ((v) >= MPT_ENUM(TypeUnrefable) \
                               && (v) < MPT_ENUM(TypeVecBase))
	
	/* object types */
	MPT_ENUM(TypeObject)    = 0x18,  /* CAN */
	MPT_ENUM(TypeSolver)    = 0x19,  /* EM */
	MPT_ENUM(TypeGroup)     = 0x1a,  /* SUB */
	MPT_ENUM(TypeOutput)    = 0x1b,  /* ESC */
#define MPT_value_isObject(v)  ((v) >= MPT_ENUM(TypeObject) \
                             && (v) < MPT_ENUM(TypeVecBase))
	
	/* vector types (0x20..0x3f) */
	MPT_ENUM(TypeVecBase)   = ' ',   /* 0x20: generic vector */
#define MPT_value_isVector(v) ((v) >= MPT_ENUM(TypeVecBase) \
                            && (v) <  MPT_ENUM(TypeArrBase))
	
	/* array types ('@'..'Z'..0x5f) */
	MPT_ENUM(TypeArrBase)   = '@',   /* 0x40: generic array */
#define MPT_value_isArray(v)  ((v) >= MPT_ENUM(TypeArrBase) \
                            && (v) < MPT_ENUM(TypeScalBase))
	
	/* scalar types ('`'..'z'..0x7f) */
	MPT_ENUM(TypeScalBase)  = '`',   /* 0x60: generic scalar */
#define MPT_value_isScalar(v) ((v) >= MPT_ENUM(TypeScalBase) \
                            && (v) <= MPT_ENUM(_TypeFinal))
	
#define MPT_value_fromVector(v) (MPT_value_isVector(v) \
                               ? (v) - MPT_ENUM(TypeVecBase) + MPT_ENUM(TypeScalBase) \
                               : 0)
#define MPT_value_fromArray(v)  (MPT_value_isArray(v) \
                               ? (v) - MPT_ENUM(TypeArrBase) + MPT_ENUM(TypeScalBase) \
                               : 0)
#define MPT_value_toVector(v)   (((v) & 0x7f) < MPT_ENUM(TypeScalBase) \
                               ? 0 \
                               : (v) - MPT_ENUM(TypeScalBase) + MPT_ENUM(TypeVecBase))
#define MPT_value_toArray(v)    (((v) & 0x7f) < MPT_ENUM(TypeScalBase) \
                               ? 0 \
                               : (v) - MPT_ENUM(TypeScalBase) + MPT_ENUM(TypeArrBase))
	MPT_ENUM(TypeFloat80)   = MPT_ENUM(TypeScalBase),   /* reuse for 80bit float value */
	
	/* range for type allocations */
	MPT_ENUM(_TypeDynamic)  = 0x80,
	MPT_ENUM(_TypeFinal)    = 0xff
};

enum MPT_ENUM(SocketFlags) {
	MPT_ENUM(SocketWrite)  = 0x1,
	MPT_ENUM(SocketRead)   = 0x2,
	MPT_ENUM(SocketRdWr)   = 0x3,
	MPT_ENUM(SocketStream) = 0x4
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

/* get/add registered (primitive) types */
extern ssize_t mpt_valsize(int);
extern int mpt_valtype_add(size_t);

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
    { return _base+length(); }
    
    inline iterator nth(int i) const
    {
        if (i > (int) length()) return 0;
        if (i < 0 && (i += length()) < 0) return 0;
        return base() + i;
    }
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
protected:
    T *_base;
    size_t _len;
};
#endif

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
	
	void * const *pointer(int = 0);
	size_t scalar(int = 0);
	const struct iovec *vector(int = 0);
	const struct array *array(int = 0);
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
	
	inline property(const char *n = "", const char *v = 0) : name(n), desc(0), val(0, v)
	{ }
	inline property(const char *n, const char *f, const void *d) : name(n), desc(0), val(f, d)
	{ }
	inline property(size_t pos) : name(0), desc((char *) pos)
	{ }
#endif
	const char *name;      /* property name */
	const char *desc;      /* property [index->]description */
	MPT_STRUCT(value) val; /* element value */
};
typedef int (*MPT_TYPE(PropertyHandler))(void *, const MPT_STRUCT(property) *);

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

/* basic unref interface */
#ifdef __cplusplus
MPT_INTERFACE(unrefable)
{
public:
	virtual void unref() = 0;
protected:
	inline ~unrefable() { }
};
#else
MPT_INTERFACE(unrefable);
MPT_INTERFACE_VPTR(unrefable)
{
	void (*unref)(MPT_INTERFACE(unrefable) *);
}; MPT_INTERFACE(unrefable) {
	const MPT_INTERFACE_VPTR(unrefable) *_vptr;
};
#endif

#ifdef __cplusplus
class Transform;

extern int convert(const void **, int , void *, int);

template<typename T>
inline __MPT_CONST_EXPR int typeIdentifier() { return static_cast<int>(T::Type); }
template<typename T>
inline __MPT_CONST_EXPR char typeIdentifier(const T &) { return static_cast<char>(typeIdentifier<T>()); }

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

template<typename T>
inline __MPT_CONST_EXPR unsigned char vectorIdentifier() {
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
    { T *ref = _ref; _ref = 0; return ref; }
protected:
    T *_ref;
};
#endif

/*! interface to send data */
#ifdef __cplusplus
MPT_INTERFACE(logger) : public unrefable
{
protected:
	inline ~logger() {}
public:
	enum { Type = TypeLogger };
	
	int message(const char *, int , const char *, ...);
	
	static logger *defaultInstance();
	
	virtual int log(const char *, int, const char *, va_list) = 0;
	
	enum {
# define MPT_LOG(x) x
#else
# define MPT_LOG(x) MPT_Log##x
enum MPT_ENUM(LogType) {
#endif
	MPT_LOG(LevelNone)      = 0x0,  /* filter messages down to ... */
	MPT_LOG(LevelCritical)  = 0x1,
	MPT_LOG(LevelError)     = 0x2,
	MPT_LOG(LevelWarning)   = 0x3,
	MPT_LOG(LevelInfo)      = 0x4,
	MPT_LOG(LevelDebug1)    = 0x5,
	MPT_LOG(LevelDebug2)    = 0x6,
	MPT_LOG(LevelDebug3)    = 0x7,
	MPT_LOG(LevelFile)      = 0x8,
	
	MPT_LOG(Message)   = 0x0,   /* user (terminal) messages */
	MPT_LOG(Fatal)     = 0x1,
	MPT_LOG(Critical)  = 0x2,
	MPT_LOG(Error)     = 0x10,
	MPT_LOG(Warning)   = 0x20,
	MPT_LOG(Info)      = 0x30,
	MPT_LOG(Debug)     = 0x40,  /* debug level types */
	MPT_LOG(Debug2)    = 0x50,
	MPT_LOG(Debug3)    = 0x60,
	MPT_LOG(Debug4)    = 0x70,
	MPT_LOG(File)      = 0x80   /* use log target */
};
enum MPT_ENUM(LogFlags)
{
	MPT_ENUM(LogPrefix)   = 0x100, /* add type prefix */
	MPT_ENUM(LogSelect)   = 0x200, /* use ANSI colouring */
	MPT_ENUM(LogANSIMore) = 0x400, /* no forced ANSI termination */
	MPT_ENUM(LogPretty)   = 0x300,
	
	MPT_ENUM(LogFunction) = 0x800  /* auto-add function decorator */
};
#ifdef __cplusplus
};
#else
MPT_INTERFACE(logger);
MPT_INTERFACE_VPTR(logger) {
	MPT_INTERFACE_VPTR(unrefable) ref;
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
MPT_INTERFACE(metatype);
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

/* reference operations */
extern uintptr_t mpt_reference_raise(MPT_STRUCT(reference) *);
extern uintptr_t mpt_reference_lower(MPT_STRUCT(reference) *);


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


/* compare data types */
extern int mpt_value_compare(const MPT_STRUCT(value) *, const void *);

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);
/* process properties according to mask */
extern int mpt_generic_foreach(int (*)(void *, MPT_STRUCT(property) *), void *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(-1));


/* create logging interface with output reference */
extern MPT_INTERFACE(logger) *mpt_output_logger(MPT_INTERFACE(output) *);
/* log output */
extern int mpt_log(MPT_INTERFACE(logger) *, const char *, int , const char *, ... );
/* get default logger instance */
extern MPT_INTERFACE(logger) *mpt_log_default(void);

/* set default logger options */
extern int mpt_log_default_format(int);
extern int mpt_log_default_skip(int);
extern int mpt_log_default_level(int);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* start log message */
extern const char *mpt_log_intro(FILE *, int, const char *);
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
std::ostream &operator<<(std::ostream &, mpt::value);
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
