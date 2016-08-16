/*!
 * MPT core library
 *  resizable data segments
 */

#ifndef _MPT_ARRAY_H
#define _MPT_ARRAY_H  @INTERFACE_VERSION@

#ifdef __cplusplus
# include <new>
# include <cstring>
struct iovec;
# include "meta.h"
#else
# include "core.h"
#endif


__MPT_NAMESPACE_BEGIN

MPT_STRUCT(slice);

MPT_INTERFACE(metatype);

/*! header for data segment */
MPT_STRUCT(buffer)
{
#ifdef __cplusplus
	void unref();
	uintptr_t addref();
protected:
	friend struct array;
	buffer();
	~buffer();
#endif
	MPT_STRUCT(buffer) *(*resize)(MPT_STRUCT(buffer) *, size_t );
	uintptr_t             shared;
	uintptr_t             size;
	uintptr_t             used;
};

/*! reference to buffer data */
MPT_STRUCT(array)
{
#ifdef __cplusplus
	enum { Type = TypeArrBase };
	
	array(array const&);
	array(size_t = 0);
	~array();
	
	size_t left() const;
	size_t length() const;
	void *base() const;
	bool shared() const;
	const Reference<buffer> &ref() const;
	
	char *string();
	bool compact();
	
	void *append(size_t , const void * = 0);
	void *prepend(size_t , size_t = 0);
	void *set(size_t , const void * = 0);
	
	int set(value);
	int set(metatype &);
	int printf(const char *fmt, ... );
	
	array & operator=  (const Reference<buffer> &);
	array & operator=  (const array &);
	array & operator=  (const slice &);
	array & operator+= (const array &);
	array & operator+= (const slice &);
	array & operator=  (struct ::iovec const&);
	array & operator+= (struct ::iovec const&);
protected:
# define _MPT_ARRAY_TYPE(x)     Array<x>
# define _MPT_REF_ARRAY_TYPE(x) RefArray<x>
#else
# define _MPT_ARRAY_TYPE(x)     MPT_STRUCT(array)
# define _MPT_REF_ARRAY_TYPE(x) MPT_STRUCT(array)
# define MPT_ARRAY_INIT { 0 }
#endif
	MPT_STRUCT(buffer) *_buf;
};
/*! transforming appended data */
MPT_STRUCT(encode_array)
{
#ifdef __cplusplus
public:
	encode_array(DataEncoder = 0);
	~encode_array();
	
	bool prepare(size_t);
	ssize_t push(size_t , const void *);
	bool setEncoding(DataEncoder);
	bool trim(size_t = 0);
	
	bool push(const struct message &);
	
	Slice<uint8_t> data() const;
protected:
#else
# define MPT_ENCODE_ARRAY_INIT { 0, MPT_ENCODE_INIT, MPT_ARRAY_INIT }
#endif
	MPT_TYPE(DataEncoder)_enc;
	MPT_STRUCT(encode_state) _state;
	MPT_STRUCT(array) _d;
};

/*! reference to buffer segment */
#ifdef __cplusplus
struct slice : array
{
	slice(const array &);
	slice(const slice &);
	slice(buffer * = 0);
	~slice();
	
	Slice<uint8_t> data() const;
	
	ssize_t write(size_t , const void *, size_t);
	int set(metatype &);
	
	bool shift(ssize_t);
	bool trim (ssize_t);
protected:
#else
MPT_STRUCT(slice)
{
# define MPT_SLICE_INIT { MPT_ARRAY_INIT, 0, 0 }
	MPT_STRUCT(array) _a;
#endif
	uintptr_t _off, /* start position on array */
	          _len; /* used data size on array */
};

#ifdef __cplusplus
template<typename MPT_RANGE_T>
# define MPT_RANGE_FCN range
#endif
#if defined(MPT_RANGE_FCN) && (defined(__cplusplus) || defined(MPT_RANGE_T))
extern void MPT_RANGE_FCN(MPT_RANGE_T range[2], int len, const MPT_RANGE_T *val, int ld)
{
	if (!len) {
#ifdef __cplusplus
		range[0] = range[1] = MPT_RANGE_T();
#else
		range[0] = range[1] = 0;
#endif
		return;
	}
	range[0] = range[1] = *val;
	
	if (!ld) return;
	
	while (--len > 0) {
		val += ld;
		if (*val < range[0]) range[0] = *val;
		if (*val > range[1]) range[1] = *val;
	}
}
#endif

#ifdef __cplusplus
template<typename MPT_COPY_ST, typename MPT_COPY_DT>
# define MPT_COPY_FCN copy
#endif
#if defined(MPT_COPY_FCN) && (defined(__cplusplus) || (defined(MPT_COPY_ST) && defined(MPT_COPY_DT)))
extern void MPT_COPY_FCN(int pts, const MPT_COPY_ST *src, int lds, MPT_COPY_DT *dest, int ldd)
{
	intptr_t i, j;
	
	if (pts <= 0) return;
	
	if (!ldd) {
		dest[0] = src[(pts-1) * lds];
		return;
	}
	if (!lds) {
		for (i = 0; i < pts; i++) {
			dest[i] = src[0];
		}
		return;
	}
	if (lds == 1 && ldd == 1) {
		for (i = 0; i < pts; i++) {
			dest[i] = src[i];
		}
		return;
	}
	for (i = 0, j = 0; pts--; i += ldd, j += lds) {
		dest[i] = src[j];
	}
}
#endif

__MPT_EXTDECL_BEGIN

/* get range of array */
extern void mpt_drange(double  *, int , const double  *, int);
extern void mpt_frange(float   *, int , const float   *, int);
extern void mpt_irange(int32_t *, int , const int32_t *, int);

/* copy/convert operations with leading dimension */
#if !defined(MPT_COPY_ST) && !defined(MPT_COPY_DT)
extern void mpt_copy64(int , const void *, int , void *, int);
extern void mpt_copy32(int , const void *, int , void *, int);

extern void mpt_copy_fd(int , const float  *, int , double *, int);
extern void mpt_copy_df(int , const double *, int , float  *, int);
#endif

/* metatype with buffer data */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(const MPT_STRUCT(array) *);

/* array manipulation */
extern size_t mpt_array_reduce(MPT_STRUCT(array) *);
extern void mpt_array_clone(MPT_STRUCT(array) *, const MPT_STRUCT(array) *);

/* add data to array */
extern void *mpt_array_append(MPT_STRUCT(array) *, size_t , const void *__MPT_DEFPAR(0));
extern void *mpt_array_insert(MPT_STRUCT(array) *, size_t , size_t);
/* remove data from array */
extern ssize_t mpt_array_cut(MPT_STRUCT(array) *, size_t , size_t);

/* get data element */
extern void *mpt_array_data(const MPT_STRUCT(array) *, size_t , size_t);

/* create and return slice data */
extern void *mpt_array_slice(MPT_STRUCT(array) *, size_t , size_t __MPT_DEFPAR(0));

/* write data to slice */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *, size_t , const void *, size_t);

/* get strings from slice */
extern int mpt_slice_conv(MPT_STRUCT(slice) *, int , void *);

/* snprintf to to array */
extern int mpt_printf(MPT_STRUCT(array) *, const char *, ... );
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__cplusplus)
extern int mpt_vprintf(MPT_STRUCT(array) *, const char *, va_list );
#endif
/* return zero-terminated buffer data */
extern char *mpt_array_string(MPT_STRUCT(array) *);

/* add message to array */
extern ssize_t mpt_array_push(MPT_STRUCT(encode_array) *, size_t len, const void *data);

/* pointer/metatype array */
extern size_t mpt_array_compact(void **, size_t);
extern size_t mpt_array_move(void *, size_t , size_t , size_t);

/* buffer resizing */
extern MPT_STRUCT(buffer) *_mpt_buffer_realloc(MPT_STRUCT(buffer) *, size_t);
extern MPT_STRUCT(buffer) *_mpt_buffer_remap  (MPT_STRUCT(buffer) *, size_t);

/* bit operations */
extern int mpt_bitmap_set(uint8_t *, size_t , size_t);
extern int mpt_bitmap_unset(uint8_t *, size_t , size_t);
extern int mpt_bitmap_get(const uint8_t *, size_t , size_t);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline array::array(size_t len) : _buf(0)
{ if (len && prepend(len)) _buf->used = 0; }
inline array::~array()
{ if (_buf) _buf->unref(); }
inline void *array::base() const
{ return _buf ? (void *) (_buf+1) : 0; }
inline size_t array::length() const
{ return _buf ? _buf->used : 0; }
inline size_t array::left() const
{ return _buf ? _buf->size - _buf->used : 0; }
inline bool array::shared() const
{ return _buf && _buf->shared; }

inline array &array::operator= (slice const& from)
{ Slice<uint8_t> d = from.data(); set(d.length(), d.base()); return *this; }
inline array &array::operator+= (array const& from)
{ append(from.length(), from.base()); return *this; }
inline array &array::operator+= (slice const& from)
{ Slice<uint8_t> d = from.data(); append(d.length(), d.base()); return *this; }

inline slice::slice(array const& a) : array(a), _off(0)
{ _len = length(); }
inline slice::~slice()
{ }
inline Slice<uint8_t> slice::data() const
{ return Slice<uint8_t>(_len ? ((uint8_t *) (_buf+1))+_off : 0, _len); }

void compact(Slice<void *> &);

template <typename T>
void move(T *v, size_t from, size_t to)
{
    if (from == to) return;
    
    T t = v[from];
    v[from].~T();
    
    if (from < to) {
        memmove((void *) (v+from), (void *) (v+from+1), (to-from)*sizeof(*v));
    } else {
        memmove((void *) (v+to+1), (void *) (v+to),     (from-to)*sizeof(*v));
    }
    v[to] = t;
}

/*! typed array with standard operations */
template <typename T>
class Array
{
public:
    typedef T* iterator;
    
    inline Array(size_t len = 0) : _d(len * sizeof(T))
    { }
    ~Array()
    { clear(); }

    inline iterator begin() const
    { return static_cast<T *>(_d.base()); }
    
    inline iterator end() const
    { return begin()+length(); }
    
    Array & operator=(const Array &a)
    {
        clear();
        _d = a._d;
        return *this;
    }
    bool set(size_t pos, T const &v)
    {
        if (pos >= length() || !detach()) return false;
        T *b = begin();
        b[pos] = v;
        return true;
    }
    bool insert(size_t pos, const T &v)
    {
        if (!_d.shared()) {
            size_t i = length();
            T *b = static_cast<T *>(_d.prepend(sizeof(*b), pos*sizeof(*b)));
            if (!b) return false;
            new (b) T(v);
            b = (T *) _d.base();
            for (; i < pos; ++i) new (b+i) T();
            return true;
        }
        size_t end, pre = length();
        array a;
        T *to;
        if (!(to = static_cast<T *>(a.set(((pos < pre ? pre : pos) + 1) * sizeof(T))))) {
            return false;
        }
        if ((end = pre) > pos) {
            pre = pos;
        }
        T *b = (T *) _d.base();
        size_t i = 0;
        for (; i < pre; ++i) {
             new (to+i) T(b[i]);
        }
        for (; i < pos; ++i) {
             new (to+i) T();
        }
        new (to+i) T(v);
        for (; i < end; ++i) {
             new ((to+1)+i) T(b[i]);
        }
        _d = a;
        return true;
    }
    inline T *get(size_t pos) const
    {
        if (pos >= length()) return 0;
        return begin()+pos;
    }
    inline size_t length() const
    { return _d.length()/sizeof(T); }
    
    inline Slice<const T> slice() const
    { return Slice<const T>(begin(), length()); }
    
    bool detach()
    {
        if (!_d.shared()) return true;
        size_t len = length();
        array a;
        T *to;
        if (!(to = static_cast<T *>(a.set(len * sizeof(T))))) return false;
        T *old = begin();
        for (size_t i = 0; i < len; ++i) {
             new (to+i) T(old[i]);
        }
        _d = a;
        return true;
    }
    void clear()
    {
        if (_d.shared()) {
            _d = array();
            return;
        }
        T *b = begin();
        for (size_t i = 0, len = length(); i < len; ++i) b[i].~T();
        _d.set(0);
    }
protected:
    struct array _d;
};
template <typename T>
inline __MPT_CONST_EXPR char typeIdentifier(Array<T>)
{ return MPT_value_toArray(typeIdentifier<T>()); }
template <typename T>
inline __MPT_CONST_EXPR char typeIdentifier(Array<const T>)
{ return typeIdentifier<Array<T> >(); }


/*! typed array with standard operations */
template <typename T>
class ItemArray : public Array<Item<T> >
{
public:
    inline ItemArray(size_t len = 0) : Array<Item<T> >(len)
    { }
    
    Item<T> *append(T *t, const char *id, int len = -1)
    {
        Item<T> *it = static_cast<Item<T> *>(Array<Item<T> >::_d.append(sizeof(*it)));
        if (!it) return 0;
        new (it) Item<T>(t);
        if (it->setName(id, len)) {
            return it;
        }
        Array<Item<T> >::_d.set(Array<Item<T> >::_d.length() - sizeof(*it));
        return 0;
    }
    bool compact()
    {
        Item<T> *space = 0;
        size_t len = 0;
        for (Item<T> *pos = Array<Item<T> >::begin(), *to = Array<Item<T> >::end(); pos != to; ++pos) {
            metatype *m = pos->pointer();
            if (!m) {
                if (!space) space = pos;
                continue;
            }
            ++len;
            if (!space) continue;
            memcpy(space, pos, sizeof(*space));
            do { ++space; } while (!(m = space->pointer()) && space < pos);
        }
        if (!space) return false;
        Array<Item<T> >::_d.set(len * sizeof(*space));
        return true;
    }
};

/*! basic pointer array */
class PointerArray
{
public:
    typedef void** iterator;
    
    inline PointerArray(size_t len = 0) : _d(len * sizeof(void*))
    { }
    inline PointerArray & operator=(const PointerArray &a)
    {
        _d = a._d;
        return *this;
    }

    inline iterator begin() const
    { return (void **) _d.base(); }
    
    inline iterator end() const
    { return ((void **) _d.base())+length(); }
    
    inline size_t length() const
    {
        return _d.length()/sizeof(void *);
    }
    bool insert(size_t , void *);
    void compact();
    
    inline void *get(size_t pos) const
    {
        if (pos >= length()) return 0;
        return ((void **) _d.base())[pos];
    }
    bool set(size_t , void *) const;
    size_t unused() const;
    ssize_t offset(const void *) const;
    bool swap(size_t p1, size_t p2) const;
    bool move(size_t p1, size_t p2) const;

protected:
    struct array _d;
};

/*! extendable array for reference types */
template <typename T>
class RefArray : public PointerArray
{
public:
    typedef Reference<T>* iterator;
    
    RefArray(size_t len = 0) : PointerArray(len)
    { }
    ~RefArray()
    {
        if (!_d.shared()) resize(0);
    }
    RefArray & operator=(const RefArray &a)
    {
        if (!_d.shared()) resize(0);
        _d = a._d;
        return *this;
    }

    inline iterator begin() const
    { return static_cast<Reference<T>*>(_d.base()); }
    
    inline iterator end() const
    { return static_cast<Reference<T>*>(_d.base())+length(); }
    
    bool set(size_t pos, T *ref) const
    {
        if (_d.shared() || pos >= length()) return false;
        T **b = static_cast<T **>(_d.base()) + pos;
        if (*b) (*b)->unref();
        *b = ref;
        return true;
    }
    inline T *get(size_t pos) const
    {
        return static_cast<T *>(PointerArray::get(pos));
    }
    T **resize(size_t len)
    {
        if (_d.shared()) return 0;
        size_t s = length();
        if (len <= s) {
            T **b = static_cast<T **>(_d.base());
            for (size_t i = len; i < s; ++i) {
                if (b[i]) b[i]->unref();
            }
        }
        return static_cast<T **>(_d.set(len * sizeof(T*)));
    }
    size_t clear(const T *ref = 0) const
    {
        T **pos = static_cast<T **>(_d.base());
        size_t elem = 0, len = length();
        if (!ref) {
            for (size_t i = 0; i < len; ++i) {
                T *match;
                if (!(match = pos[i])) continue;
                match->unref();
                pos[i] = 0;
                ++elem;
            }
            return elem;
        }
        for (size_t i = 0; i < len; ++i) {
            T *match;
            if (!(match = pos[i]) || (match != ref)) continue;
            match->unref();
            pos[i] = 0;
            ++elem;
        }
        return elem;
    }
    inline const Array<Reference<T> > &asArray() const
    { return *reinterpret_cast<const Array<Reference<T> >*>(this); }
};

class LogStore : public logger
{
public:
    enum {
        PassUnsaved =   0x1,
        PassSaved   =   0x2,
        PassFile    =   0x4,
        PassMessage =   0x8,
        PassFlags   =   0xf,
        
        SaveMessage =  0x10,
        SaveLog     =  0x20,
        SaveLogAll  =  0x40,
        SaveFlags   =  0xf0,
        
        FlowNormal  = PassUnsaved | PassSaved | PassFile
    };
    LogStore(logger * = logger::defaultInstance());
    virtual ~LogStore();
    
    class Entry;
    
    void unref() __MPT_OVERRIDE;
    int log(const char *, int, const char *, va_list) __MPT_OVERRIDE;
    
    virtual const Entry *nextEntry();
    virtual void clearLog();
    
    virtual bool setIgnoreLevel(int);
    virtual bool setFlowFlags(int);
    
    inline int flowFlags() const
    { return _flags; }
    
    inline const Slice<const Entry> logEnries() const
    { return _msg.slice(); }
    
protected:
    Array<Entry> _msg;
    logger  *_next;
    uint32_t _act;
    uint8_t  _flags;
    uint8_t  _ignore;
    uint8_t  _level;
};
/*! storage for log messages */
class LogStore::Entry : ::mpt::array
{
public:
    int type() const;
    const char *source() const;
    Slice<const char> data(int part = 0) const;
    int set(const char *, int, const char *, va_list);
protected:
    struct Header
    {
        uint8_t from;
        uint8_t args;
        uint8_t _cmd;
        uint8_t type;
    };
};

/*! linear search map type */
template <typename K, typename V>
class Map
{
public:
    class Element
    {
    public:
        inline Element(const K &k = K(), const V &v = V()) : key(k), value(v)
        { }
        K key;
        V value;
    };
    typedef const Element * const_iterator;
    
    inline const_iterator begin() const
    { return _d.begin(); }
    inline const_iterator end() const
    { return _d.end(); }
    
    bool set(const K &key, const V &value)
    {
        V *d = get(key);
        if (!d) return _d.insert(_d.length(), Element(key, value));
        *d = value;
        return true;
    }
    bool append(const K &key, const V &value)
    {
        return _d.insert(_d.length(), Element(key, value));
    }
    V *get(const K &key) const
    {
        for (Element *c = _d.begin(), *e = _d.end(); c < e; ++c) {
            if (c->key == key) return &e->value;
        }
        return 0;
    }
    inline Array<V> values(const K &key) const
    {
        return values(&key);
    }
    Array<V> values(const K *key = 0) const
    {
        Array<V> a;
        for (Element *c = _d.begin(), *e = _d.end(); c < e; ++c) {
            if (!key || c->key == *key) a.insert(a.length(), c->value);
        }
        return a;
    }
protected:
    Array<Element> _d;
};

#endif /* __cplusplus */

__MPT_EXTDECL_BEGIN
/* clear references on array data (requires template in C++ mode) */
extern void mpt_array_callunref(_MPT_REF_ARRAY_TYPE(unrefable) *);
__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_ARRAY_H */
