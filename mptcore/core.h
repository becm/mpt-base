/*!
 * MPT core library
 *  basic types and definitions
 */

#ifndef _MPT_CORE_H
#define _MPT_CORE_H  201502

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
# include <string>
# include <sstream>
# include <stdlib.h>

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
# define MPT_TYPE(t)           MPT##t
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
	/* system types */
	MPT_ENUM(TypeAddress)  = 0x1,   /* SOH */
	MPT_ENUM(TypeSocket)   = 0x2,   /* STX */
	
	/* data copy types */
	MPT_ENUM(TypeValue)    = 0x4,   /* EOT */
	MPT_ENUM(TypeProperty) = 0x5,   /* ENQ */
	
	/* data pointer types */
	MPT_ENUM(TypeNode)     = 0x6,   /* ACK */
	MPT_ENUM(TypeArray)    = 0x7,   /* BEL '\a' */
	
	/* layout types */
	MPT_ENUM(TypeLineAttr) = 0x8,   /* BS  '\b' */
	MPT_ENUM(TypeLine)     = 0x9,   /* HT  '\t' */
	/* layout pointer types */
	MPT_ENUM(TypeAxis)     = 0xa,   /* LF  '\n' */
	MPT_ENUM(TypeGraph)    = 0xb,   /* BS  '\b' */
	MPT_ENUM(TypeWorld)    = 0xc,   /* FF  '\f' */
	MPT_ENUM(TypeText)     = 0xd,   /* CR  '\r' */
	
	/* reference types */
	MPT_ENUM(TypeGroup)    = 0x10,  /* DLE */
	MPT_ENUM(TypeIODevice) = 0x11,  /* DC1 */
	MPT_ENUM(TypeInput)    = 0x12,  /* DC2 */
	MPT_ENUM(TypeLogger)   = 0x13,  /* DC3 */
	MPT_ENUM(TypeCycle)    = 0x14,  /* DC4 */
	
	/* metatype and extensions */
	MPT_ENUM(TypeMeta)     = 0x18,  /* CAN */
	MPT_ENUM(TypeOutput)   = 0x19,  /* EM  */
	MPT_ENUM(TypeSolver)   = 0x1a,  /* SUB */
	
	/* primitive types with printable representation (0x20..0x7f) */
	MPT_ENUM(TypeColor)    = '#',   /* rgba(0..255) */
	
	/* vector format flag, make typed version via (0x80 | <typeid>) for builtin types (0x01..0x7f) */
	MPT_ENUM(TypeVector)   = 0x80,
	
	MPT_ENUM(TypeUser)     = 0x100,
	MPT_ENUM(_TypeFinal)   = 0x7fffffff
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
	MPT_ENUM(LogFile)      = 0x80   /* use log target */
};

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
	inline value(const char *f = 0, const void *v = 0) : fmt(f), ptr(v)
	{ }
	inline void set(const struct value &v)
	{ fmt = v.fmt; ptr = v.ptr; }
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
typedef int (*MPT_TYPE(PropertyHandler))(void *, MPT_STRUCT(property) *);

#ifdef __cplusplus
class Transform;

extern int convert(const void **, int , void *, int);

template<typename T>
inline int typeIdentifier(void) { return T::Type; }

/* floating point values */
template<> inline int typeIdentifier<float>(void)  { return 'f'; }
template<> inline int typeIdentifier<double>(void) { return 'd'; }
/* integer values */
template<> inline int typeIdentifier<int8_t>(void)  { return 'b'; }
template<> inline int typeIdentifier<int16_t>(void) { return 'h'; }
template<> inline int typeIdentifier<int32_t>(void) { return 'i'; }
template<> inline int typeIdentifier<int64_t>(void) { return 'l'; }
/* unsigned values */
template<> inline int typeIdentifier<uint8_t>(void)  { return 'B'; }
template<> inline int typeIdentifier<uint16_t>(void) { return 'H'; }
template<> inline int typeIdentifier<uint32_t>(void) { return 'I'; }
template<> inline int typeIdentifier<uint64_t>(void) { return 'L'; }

/*! container for reference type pointer */
template<typename T>
class Reference
{
public:
    Reference(T *ref = 0)
    { _ref = ref; }
    Reference(const Reference &ref) : _ref(0)
    { *this = ref; }
    ~Reference()
    { if (_ref) _ref->unref(); }
    
    operator T*() const
    { return _ref; }
    
    bool setReference(Reference const &ref)
    {
        if (_ref == ref._ref) return true;
        T *t = 0;
        if (ref._ref && !(t = ref._ref->addref())) {
            return false;
        }
        if (_ref) { _ref->unref(); _ref = 0; }
        _ref = t;
        return true;
    }
    inline Reference & operator= (Reference const &ref)
    {
        setReference(ref);
        return *this;
    }
    T *detach()
    { T *ref = _ref; _ref = 0; return ref; }
protected:
    T *_ref;
};

/*! container for message reference */
class Output
{
public:
    class Message
    {
    public:
        inline Message(int t) : _ref(1), type(t), space(1) { }

        Message *addref();
        int unref();

    protected:
        friend class Output;
        std::stringstream buf;
        uint32_t _ref;
        int16_t type;
        uint8_t space;
        uint8_t _pad;
    };
    inline Output(int type = -1) : _msg(new Message(type)) { }

    inline Output &space()    { Message *m = _msg; if (!m->space) m->buf << ' '; m->space = 1; return *this; }
    inline Output &nospace()  { Message *m = _msg; m->space = 0; return *this; }
    inline Output &maySpace() { Message *m = _msg; if (m->space) m->buf << ' '; return *this; }

    inline std::stringstream &buf() { Message *m = _msg; return m->buf; }

    template<typename T>
    inline Output &operator<<(const T &v) { buf() << v; return maySpace(); }

protected:
    Reference<Message> _msg;
};
/* output interfaces */
Output debug   (const char *fname = 0, const char *nspace = 0);
Output warning (const char *fname = 0, const char *nspace = 0);
Output critical(const char *fname = 0, const char *nspace = 0);
#endif

/*! interface to send data */
MPT_INTERFACE(logger)
#ifdef __cplusplus
{
protected:
	inline ~logger() {}
public:
	enum { Type = TypeLogger };
	
	int error(const char *, const char *, ... );
	int critical(const char *, const char *, ... );
	int warning(const char *, const char *, ... );
	
	static logger *defaultInstance(void);
	
	virtual int unref() = 0;
	virtual int log(const char *, int, const char *, va_list) = 0;
#else
; MPT_INTERFACE_VPTR(logger) {
	int (*unref)(MPT_INTERFACE(logger) *);
	int (*log)  (MPT_INTERFACE(logger) *, const char *, int , const char *, va_list);
}; MPT_INTERFACE(logger) {
	const MPT_INTERFACE_VPTR(logger) *_vptr;
# define MPT_LOGGER(l) ((MPT_INTERFACE(logger) *) ((l) ? (l)->_vptr->typecast((l), MPT_ENUM(TypeLogger)) : 0))
#endif
};

/*! interface to retrieve data elements */
MPT_INTERFACE(source)
#ifdef __cplusplus
{
protected:
	inline ~source() {}
public:
	virtual int conv(int type, void * = 0) = 0;
#else
; MPT_INTERFACE_VPTR(source) {
	int (*conv) (MPT_INTERFACE(source) *, int type, void *);
}; MPT_INTERFACE(source) {
	const MPT_INTERFACE_VPTR(source) *_vptr;
#endif
};

/*! generic metatype interface */
MPT_INTERFACE(metatype)
#ifdef __cplusplus
{
protected:
	inline ~metatype() {}
public:
	enum { Type = TypeMeta };
	
	inline int type(void)
	{ return property(0); }
	
	const char *cast(void);
	
	template <typename T>
	inline T *cast(void)
	{ return (T*) typecast(typeIdentifier<T>()); }
	
	bool set(const struct property &, logger * = logger::defaultInstance());
	bool setProperties(metatype &, logger * = logger::defaultInstance());
	
	static metatype *create(size_t size);
	
	virtual int unref() = 0;
	virtual metatype *addref() = 0;
	virtual int property(struct property *, source * = 0) = 0;
	virtual void *typecast(int) = 0;
#else
; MPT_INTERFACE_VPTR(metatype)
{
	int (*unref)(MPT_INTERFACE(metatype) *);
	MPT_INTERFACE(metatype) *(*addref)(MPT_INTERFACE(metatype) *);
	int (*property)(MPT_INTERFACE(metatype) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);
	void *(*typecast)(MPT_INTERFACE(metatype) *, int);
}; MPT_INTERFACE(metatype) {
	const MPT_INTERFACE_VPTR(metatype) *_vptr;
#endif
};

#ifdef __cplusplus
class Metatype : public metatype
{
public:
    int unref();
    Metatype *addref();
    int property(struct property *, source * = 0);
    
protected:
    Metatype(uintptr_t initref = 1);
    virtual ~Metatype();
    uintptr_t _ref;
};

/*! reduced slice with type but no data reference */
template <typename T>
class Slice
{
public:
    inline Slice(T *a, size_t len) : _base(len ? a : 0), _len(len*sizeof(T))
    { }
    inline size_t len() const
    { return _len / sizeof(T); }
    inline T *base() const
    { return _base; };
    bool skip(size_t len)
    {
        if (len > _len/sizeof(T)) return false;
        if (!(_len -= len * sizeof(T))) _base = 0;
        else _base += len;
        return true;
    }
    bool trim(size_t len)
    {
        if ((len *= sizeof(T)) > _len) return false;
        if (!(_len -= len)) _base = 0;
        return true;
    }
    const char *fmt(void)
    {
        static const char fmt[2] = {(TypeVector | typeIdentifier<T>())};
        return fmt;
    }
protected:
    T *_base;
    size_t _len;
};
#endif

/* text identifier for entity */
MPT_STRUCT(identifier)
{
#ifdef __cplusplus
	identifier(size_t = sizeof(identifier));
	inline ~identifier()
	{ setName(0, 0); }
	
	bool equal(const char *, int) const;
	Slice<const char> data(void) const;
	const char *name(void) const;
	
	bool setName(const char *, int = -1);
	bool setName(size_t , const void *);
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
    inline int unref()
    { delete this; return 0; }
    
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
    const Relation *_parent;
};
/*! interface to generic groups of metatypes elements */
class Group
{
public:
    enum { Type = TypeGroup };
    
    virtual int unref() = 0;
    
    virtual const Item<metatype> *item(size_t pos) const;
    virtual Item<metatype> *append(metatype *);
    virtual bool clear(const metatype * = 0);
    virtual bool bind(const Relation &from, logger * = logger::defaultInstance());
    virtual ssize_t offset(const metatype *) const;
    
    virtual const Transform &transform();
    
    bool copy(const Group &from, logger * = 0);
    bool addItems(node *head, const Relation *from = 0, logger * = logger::defaultInstance());
    
protected:
    virtual metatype *create(const char *, int = -1);
    virtual bool set(const property &, logger * = logger::defaultInstance());
};
/*! Relation implemetation using Group as current element */
class GroupRelation : public Relation
{
public:
    inline GroupRelation(const Group &g, const Relation *p = 0, char sep = ':') : Relation(p), _curr(g), _sep(sep)
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
	
	inline bool active(void) const
	{ return _id >= 0; }
	
	bool bind(const char *, int = 2);
	bool open(const char *, const char *mode = "w");
	
	bool set(source &);
protected:
#else
# define MPT_SOCKET_INIT       { -1 }
# define MPT_socket_active(s)  ((s)->_id >= 0)
#endif
	int32_t  _id;     /* message descriptor */
};
#ifdef __cplusplus
class Stream;
class Socket : public Metatype, public socket
{
public:
    Socket(socket * = 0, uintptr_t = 1);
    virtual ~Socket();
    
    enum { Type = socket::Type };
    
    Socket *addref();
    int property(struct property *, source * = 0);
    void *typecast(int);
    
    virtual Reference<class Stream> accept();
};
#endif

__MPT_EXTDECL_BEGIN

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
extern void mpt_identifier_init(MPT_STRUCT(identifier) *, size_t);
extern const void *mpt_identifier_set(MPT_STRUCT(identifier) *, const char *, int);


/* create meta type element */
extern MPT_INTERFACE(metatype) *mpt_meta_new(size_t);
/* new generic metatype with same data and identifier */
extern MPT_INTERFACE(metatype) *mpt_meta_clone(MPT_INTERFACE(metatype) *);

/* loop trough metatype/generic properties */
extern int mpt_meta_foreach(MPT_INTERFACE(metatype) *, MPT_TYPE(PropertyHandler) , void *, int __MPT_DEFPAR(0));

/* get matching property by name */
extern int mpt_property_match(const char *, int , const MPT_STRUCT(property) *, size_t);

/* set metatype property to match argument */
extern int mpt_meta_pset(MPT_INTERFACE(metatype) *, MPT_STRUCT(property) *, int (*)(const char **, int ,void *) __MPT_DEFPAR(0));
extern int mpt_meta_vset(MPT_INTERFACE(metatype) *, const char *, const char *, va_list);
extern int mpt_meta_set (MPT_INTERFACE(metatype) *, const char *, const char *, ... );
/* get node/metatype text/raw data */
extern const void *mpt_meta_data(MPT_INTERFACE(metatype) *, size_t *__MPT_DEFPAR(0));
/* get metatype name */
extern const char *mpt_meta_typename(MPT_INTERFACE(metatype) *);


/* initialize geninfo data */
extern int _mpt_geninfo_init(void *, size_t , uint32_t __MPT_DEFPAR(1));
/* reference operations on geninfo data */
extern uint32_t _mpt_geninfo_unref(uint64_t *);
extern uint32_t _mpt_geninfo_addref(uint64_t *);
/* property operations on geninfo data */
extern int _mpt_geninfo_property(uint64_t *, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *);


/* log output */
extern int mpt_log(MPT_INTERFACE(logger) *, const char *, int , const char *, ... );

/* write error message and abort program */
extern void _mpt_abort(const char *, const char *, const char *, int);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CORE_H */
