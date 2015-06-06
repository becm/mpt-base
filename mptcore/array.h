/*!
 * MPT core library
 *  resizable data segments
 */

#ifndef _MPT_ARRAY_H
#define _MPT_ARRAY_H  201502

#include "core.h"

#ifdef __cplusplus
# include <cstring>
struct iovec;
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(slice);

/*! header for data segment */
MPT_STRUCT(buffer)
{
#ifdef __cplusplus
	int unref();
	buffer *addref();
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
	enum { Type = TypeArray };
	
	array(array const&);
	array(size_t = 0);
	~array();
	
	size_t left() const;
	size_t used() const;
	void *base() const;
	bool shared(void) const;
	Reference<buffer> ref() const;
	
	char *string();
	bool compact();
	
	void *append (size_t , const void * = 0);
	void *prepend(size_t , size_t = 0);
	void *set    (size_t , const void * = 0);
	
	int set(source &);
	int printf(const char *fmt, ... );
	
	array & operator=  (const array &);
	array & operator=  (const slice &);
	array & operator+= (const array &);
	array & operator+= (const slice &);
	array & operator=  (struct ::iovec const&);
	array & operator+= (struct ::iovec const&);
protected:
#else
# define MPT_ARRAY_INIT { 0 }
#endif
	MPT_STRUCT(buffer) *_buf;
};

/*! reference to buffer segment */
#ifdef __cplusplus
struct slice : array
{
	slice(array const& );
	slice(slice const& );
	slice(buffer * = 0);
	~slice();
	
	Slice<uint8_t> data() const;
	
	ssize_t write(size_t , const void *, size_t);
	int set(source &);
	
	bool shift(ssize_t);
	bool trim (ssize_t);
protected:
#else
MPT_STRUCT(slice)
{
# define MPT_SLICE_INIT { MPT_ARRAY_INIT, 0, 0 }
	MPT_STRUCT(array) _a;
#endif
	uintptr_t         _off,
	                  _len;
};

#ifdef __cplusplus
template<typename MPT_RANGE_T, typename MPT_ARRAY_T>
#endif
#if defined(__cplusplus) || (defined(range) && defined(MPT_RANGE_T) && defined(MPT_ARRAY_T))
extern void range(MPT_RANGE_T *range, int len, const MPT_ARRAY_T *val, int ld)
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
#endif
#if defined(__cplusplus) || (defined(copy) && defined(MPT_COPY_ST) && defined(MPT_COPY_DT))
extern void copy(int pts, const MPT_COPY_ST *src, int lds, MPT_COPY_DT *dest, int ldd)
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
extern void mpt_copy64 (int , const void *, int , void *, int);
extern void mpt_copy32 (int , const void *, int , void *, int);

extern void mpt_copy_fd(int , const float  *, int , double *, int);
extern void mpt_copy_df(int , const double *, int , float  *, int);
#endif

/* metatype with buffer data */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(size_t , const void *);

/* array manipulation */
extern size_t mpt_array_reduce(MPT_STRUCT(array) *);
extern void mpt_array_clone(MPT_STRUCT(array) *, const MPT_STRUCT(array) *);

/* add data to array */
extern void *mpt_array_append(MPT_STRUCT(array) *, size_t , const void *__MPT_DEFPAR(0));
extern void *mpt_array_insert(MPT_STRUCT(array) *, size_t , size_t);
/* remove data from array */
extern ssize_t mpt_array_cut(MPT_STRUCT(array) *, size_t , size_t);

/* get data element */
extern void *mpt_array_data (const MPT_STRUCT(array) *, size_t , size_t);

/* create and return slice data */
extern void *mpt_array_slice(MPT_STRUCT(array) *, size_t , size_t __MPT_DEFPAR(0));

/* write data to slice */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *, size_t , const void *, size_t);

/* clear references on array data */
extern void mpt_array_callunref(const MPT_STRUCT(array) *);

/* snprintf to to array */
extern int mpt_printf(MPT_STRUCT(array) *, const char *, ... );
#if (defined(_VA_LIST) || defined(_VA_LIST_DECLARED)) && (!defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L)
extern int mpt_vprintf(MPT_STRUCT(array) *, const char *, va_list );
#endif
/* return zero-terminated buffer data */
extern char *mpt_array_string(MPT_STRUCT(array) *);

/* add message to array */
extern ssize_t mpt_array_push(MPT_STRUCT(array) *, MPT_STRUCT(codestate) *, MPT_TYPE(DataEncoder), const struct iovec *);

/* write/send array data */
#if defined(_SYS_SOCKET_H) || defined(_SYS_SOCKET_H_)
extern ssize_t mpt_array_flush(int , const MPT_STRUCT(slice) *, const struct sockaddr *, socklen_t);
#endif

/* pointer/metatype array */
extern size_t mpt_array_compact(void **, size_t );
extern size_t mpt_array_move   (void *, size_t , size_t , size_t);

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
inline size_t array::used() const
{ return _buf ? _buf->used : 0; }
inline size_t array::left() const
{ return _buf ? _buf->size - _buf->used : 0; }
inline bool array::shared(void) const
{ return _buf && _buf->shared; }

/*! transtorming appended data */
class EncodingArray
{
public:
    EncodingArray(DataEncoder = 0);
    ~EncodingArray();
    
    ssize_t push(size_t , const void *);
    bool setData(const struct message *);
    bool setEncoding(DataEncoder);
    bool trim(size_t = 0);
    
    Slice<uint8_t> data() const;
    
protected:
    DataEncoder _enc;
    codestate _state;
    array _d;
};

inline array &array::operator= (slice const& from)
{ Slice<uint8_t> d = from.data(); set(d.len(), d.base()); return *this; }
inline array &array::operator+= (array const& from)
{ append(from.used(), from.base()); return *this; }
inline array &array::operator+= (slice const& from)
{ Slice<uint8_t> d = from.data(); append(d.len(), d.base()); return *this; }

inline slice::slice(array const& a) : array(a), _off(0)
{ _len = used(); }
inline slice::~slice()
{ }
inline Slice<uint8_t> slice::data() const
{ return Slice<uint8_t>(_len ? ((uint8_t *) (_buf+1))+_off : 0, _len); }


#ifdef _MPT_QUEUE_H
/* IO extension to buffer */
class Buffer : public Metatype, public IODevice, public EncodingArray
{
public:
    enum { Type = TypeIODevice };
    Buffer();
    ~Buffer();
    
    Buffer *addref(void);
    int unref(void);
    int property(struct property *, source * = 0);
    void *typecast(int);
    
    ssize_t read(size_t , void *, size_t = 1);
    ssize_t write(size_t , const void *, size_t = 1);
    
    Slice<uint8_t> peek(size_t);
    bool seek(int64_t);
    int64_t pos();
};
#endif

size_t compact(void **, size_t);

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
    class iterator
    {
    public:
        iterator(T *d) : _d(d)
        { }
        iterator operator ++()
        { ++_d; return *this; }
        const T & operator *() const
        { return *_d; }
        bool operator ==(const iterator &it)
        { return _d == it._d; }
        bool operator !=(const iterator &it)
        { return !(*this == it); }
    protected:
        T *_d;
    };
    
    inline Array(size_t len = 0) : _d(len * sizeof(T))
    { }
    ~Array()
    { clear(); }

    inline iterator begin() const
    { return iterator(get(0)); }
    
    inline iterator end() const
    { return iterator(get(0)+size()); }
    
    Array & operator=(const Array &a)
    {
        clear();
        _d = a._d;
        return *this;
    }
    bool set(size_t pos, T const &v)
    {
        if (pos >= size() || !detach()) return false;
        T *b = (T *) _d.base();
        b[pos] = v;
        return true;
    }
    bool insert(size_t pos, const T &v)
    {
        if (!_d.shared()) {
            size_t i = size();
            T *b = (T *) _d.prepend(sizeof(*b), pos*sizeof(*b));
            if (!b) return false;
            new (b) T(v);
            b = (T *) _d.base();
            for (; i < pos; ++i) new (b+i) T();
            return true;
        }
        size_t end, pre = size();
        array a;
        T *to;
        if (!(to = (T *) a.set(((pos < pre ? pre : pos) + 1) * sizeof(T)))) {
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
        if (pos >= size()) return 0;
        return ((T*) _d.base())+pos;
    }
    inline size_t size() const
    { return _d.used()/sizeof(T); }
    
    inline Slice<const T> slice() const
    { return Slice<const T>(get(0), size()); }
    
    bool detach()
    {
        if (!_d.shared()) return true;
        size_t len = size();
        array a;
        T *to;
        if (!(to = a.set(len * sizeof(T)))) return false;
        T *old = (T *) _d.base();
        for (size_t i = 0; i < len; ++i) {
             new (to+i) T(old[i]);
        }
        _d = a;
        return true;
    }
    void clear(void)
    {
        if (_d.shared()) {
            _d = array();
            return;
        }
        T *b = (T *) _d.base();
        for (size_t i = 0, len = size(); i < len; ++i) b[i].~T();
        _d.set(0);
    }
protected:
    struct array _d;
};

/*! storage for log messages */
class LogEntry : array
{
public:
    int type(void) const;
    const char *source(void) const;
    Slice<const char> data(int part = 0) const;
    int set(const char *, int, const char *, va_list );
protected:
    struct Header
    {
        uint8_t from;
        uint8_t args;
        uint8_t _cmd;
        uint8_t type;
    };
};
class LogStore : public logger
{
public:
    LogStore();
    virtual ~LogStore();
    
    int unref();
    int log(const char *, int, const char *, va_list);
    
    virtual const LogEntry *nextEntry(void);
    virtual void clearLog(void);
    
    virtual bool setStoreRange(int, int);
    virtual bool setPassRange(int, int);
    
    inline const Slice<const LogEntry> logEnries(void) const
    { return _msg.slice(); }
    
protected:
    Array<LogEntry> _msg;
    logger *_next;
    uint32_t _act;
    struct {
        uint8_t min, max;
    } _store;
    struct {
        uint8_t min, max;
    } _pass;
};

/*! linear search map type */
template <typename K, typename V>
class Map
{
public:
    bool set(const K &key, const V &value)
    {
        V *d = get(key);
        if (!d) return _d.insert(_d.size(), Element(key, value));
        *d = value;
        return true;
    }
    inline bool append(const K &key, const V &value)
    {
        return _d.insert(_d.size(), Element(key, value));
    }
    V *get(const K &key) const
    {
        Element *e = _d.get(0);
        size_t len = _d.size();
        for (size_t i = 0; i < len; ++i, ++e) {
            if (e->key == key) return &e->value;
        }
        return 0;
    }
    inline Array<V> values(const K &key) const
    {
        return values(&key);
    }
    inline Array<V> values() const
    {
        return values(0);
    }
protected:
    class Element
    {
    public:
        Element(const K &k, const V &v) : key(k), value(v)
        { }
        Element()
        { }
        ~Element()
        { }
        K key;
        V value;
    };
    Array<V> values(const K *key) const
    {
        Array<V> a;
        Element *e = _d.get(0);
        size_t len = _d.size();
        for (size_t i = 0; i < len; ++i, ++e) {
            if (!key || e->key == *key) a.insert(a.size(), e->value);
        }
        return a;
    }
    Array<Element> _d;
};

/*! extendable array for reference types */
template <typename T>
class RefArray
{
public:
    RefArray(size_t len = 0) : _d(len * sizeof(T*))
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
    bool set(size_t pos, T *ref) const
    {
        if (_d.shared() || pos >= size()) return false;
        T **b = ((T **) _d.base()) + pos;
        if (*b) (*b)->unref();
        *b = ref;
        return true;
    }
    bool insert(size_t pos, T *ref)
    {
        T **b;
        if (_d.shared() || !(b = (T **) _d.prepend(sizeof(*b), pos*sizeof(*b)))) return false;
        *b = ref;
        return true;
    }
    inline T *get(size_t pos) const
    {
        if (pos >= size()) return 0;
        return ((T**) _d.base())[pos];
    }
    T **resize(size_t len)
    {
        if (_d.shared()) return 0;
        size_t s = size();
        if (len <= s) {
            T **b = (T**) _d.base();
            for (size_t i = len; i < s; ++i) {
                if (b[i]) b[i]->unref();
            }
        }
        return (T**) _d.set(len * sizeof(T*));
    }
    size_t clear(const T *ref = 0) const
    {
        T **pos = (T**) _d.base();
        size_t elem = 0, len = size();
        if (!ref) {
            for (size_t i = 0; i < len; ++i) {
                if (!pos[i]) ++elem;
            }
            return elem;
        }
        for (size_t i = 0; i < len; ++i) {
            if (!pos[i] || (pos[i] != ref)) continue;
            pos[i]->unref();
            pos[i] = 0;
            ++elem;
        }
        return elem;
    }
    ssize_t offset(const T *ref) const
    {
        T **pos = (T**) _d.base();
        size_t len = size();
        for (size_t i = 0; i < len; ++i) {
             if (ref == pos[i]) return i;
        }
        return -1;
    }
    void compact()
    {
        size_t len = ::mpt::compact((void **) _d.base(), size());
        _d.set(len * sizeof(T*));
    }
    bool swap(size_t p1, size_t p2) const
    {
        size_t len = size();
        if (p1 > len || p2 > len) return false;
        T *t, **b = (T **) _d.base();
        t = b[p1];
        b[p1] = b[p2];
        b[p2] = t;
        return true;
    }
    bool move(size_t p1, size_t p2) const
    {
        size_t len = size();
        if (p1 >= len || p2 >= len) return false;
        ::mpt::move((T**) _d.base(), p1, p2);
        return true;
    }
    inline size_t size() const
    { return _d.used()/sizeof(T *); }

    inline const Array<Reference<T> > &toArray() const
    { return *((const Array<Reference<T> > *) this); }
    
protected:
    struct array _d;
};

/*! Group implementation using reference array */
class Collection : public Group
{
public:
    virtual ~Collection();
    
    const Item<metatype> *item(size_t) const;
    Item<metatype> *append(metatype *);
    bool clear(const metatype * = 0);
    bool bind(const Relation &, logger * = logger::defaultInstance());
    ssize_t offset(const metatype *) const;
    
protected:
    int unref();
    RefArray<Item<metatype> > _items;
};

#endif /* __cplusplus */

__MPT_NAMESPACE_END

#endif /* _MPT_BUFFER_H */
