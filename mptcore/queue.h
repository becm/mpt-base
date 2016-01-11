/*!
 * MPT core library
 *  queue operations on managed data
 */

#ifndef _MPT_QUEUE_H
#define _MPT_QUEUE_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(message);

MPT_STRUCT(queue)
{
#ifdef __cplusplus
	inline queue() : base(0), len(0), max(0), off(0)
	{ }
	inline bool fragmented()
	{ return (max - len) < off; }
#else
# define MPT_QUEUE_INIT    { 0, 0, 0, 0 }
# define MPT_queue_frag(q) (((q)->max - (q)->len) < (q)->off)
#endif
	void   *base;  /* data start address */
	size_t  len,   /* queue used length */
	        max,   /* max. data size */
	        off;   /* queue start offset */
};

__MPT_EXTDECL_BEGIN

/* switch memory before and after pivot */
extern void *mpt_memrev(void *, size_t , size_t);
/* switch memory content */
extern void *mpt_memswap(void *, void *, size_t);

/* get address/range of empty parts */
extern void *mpt_queue_empty(const MPT_STRUCT(queue) *, size_t *, size_t *);
extern void *mpt_queue_data (const MPT_STRUCT(queue) *, size_t *);

/* reallocate queue data to new size */
extern void *mpt_queue_resize(MPT_STRUCT(queue) *, size_t);
extern size_t mpt_queue_prepare(MPT_STRUCT(queue) *, size_t);

/* set/get data at position (split data needs target/source pointer) */
extern int mpt_queue_set(const MPT_STRUCT(queue) *, size_t , size_t , const void *);
extern int mpt_queue_get(const MPT_STRUCT(queue) *, size_t , size_t , void *);

extern char *mpt_queue_string(MPT_STRUCT(queue) *);

/* find matching element in queue */
extern void *mpt_queue_find(const MPT_STRUCT(queue) *, size_t , int (*)(const void *, void *) , void *);

/* add/cut/query data at right side */
extern int mpt_qpush(MPT_STRUCT(queue) *, size_t , const void *);
extern void *mpt_qpop(MPT_STRUCT(queue) *, size_t , void *);

/* add/cut/query data at left side */
extern int mpt_qunshift(MPT_STRUCT(queue) *, size_t , const void *);
extern void *mpt_qshift(MPT_STRUCT(queue) *, size_t , void *);

/* remove specific part from queue (offset, length) */
extern int mpt_queue_crop(MPT_STRUCT(queue) *, size_t , size_t);

/* prepare data area at end/beginning of queue */
extern ssize_t mpt_qpost(MPT_STRUCT(queue) *, size_t);
extern ssize_t mpt_qpre(MPT_STRUCT(queue) *, size_t);

/* read/write operation on file descriptor */
extern ssize_t mpt_queue_load(MPT_STRUCT(queue) *, int , size_t);
extern ssize_t mpt_queue_save(MPT_STRUCT(queue) *, int);

/* align queue data, return implicit buffer view if offset == 0 */
extern void mpt_queue_align(MPT_STRUCT(queue) *, size_t);

/* get next message */
extern ssize_t mpt_queue_peek(const MPT_STRUCT(queue) *, MPT_STRUCT(codestate) *, MPT_TYPE(DataDecoder), const struct iovec *);
extern ssize_t mpt_queue_recv(const MPT_STRUCT(queue) *, MPT_STRUCT(codestate) *, MPT_TYPE(DataDecoder));
/* send message */
extern ssize_t mpt_queue_push(MPT_STRUCT(queue) *, MPT_TYPE(DataEncoder), MPT_STRUCT(codestate) *, struct iovec *);

__MPT_EXTDECL_END

#ifdef __cplusplus

/*!  */
class DecodingQueue
{
public:
    DecodingQueue(DataDecoder);
    ~DecodingQueue();
    
    bool pendingMessage();
    bool currentMessage(message &, struct iovec * = 0);
    bool advance();

protected:
    DataDecoder _dec;
    codestate _state;
    ssize_t _mlen;
    queue _d;
};

/*! interface to raw device */
class IODevice
{
public:
    enum { Type = TypeIODevice };
    
    virtual void unref() = 0;
    virtual ssize_t write(size_t , const void *, size_t = 1) = 0;
    virtual ssize_t read(size_t , void *, size_t = 1) = 0;
    
    virtual int64_t pos();
    virtual bool seek(int64_t);
    virtual Slice<uint8_t> peek(size_t);
    virtual int getchar();
    
protected:
    virtual ~IODevice()
    { }
};

class Queue : public metatype, public IODevice
{
public:
    Queue(size_t = 0);
    ~Queue();
    
    enum { Type = IODevice::Type };
    
    /* metatype interface */
    void unref();
    int assign(const value *);
    int conv(int, void *);
    
    /* IODevice interface */
    ssize_t write(size_t , const void *, size_t);
    ssize_t read(size_t , void *, size_t);
    
    Slice<uint8_t> data();
    
    /* queue access */
    virtual bool prepare(size_t);
    
    virtual bool push(const void *, size_t);
    virtual bool pop(void *, size_t);
    
    virtual bool unshift(const void *, size_t);
    virtual bool shift(void *, size_t);
    
protected:
    struct queue _d;
};

template <typename T>
class Pipe : Reference<Queue>
{
public:
    Pipe(const T &v, size_t len)
    {
        if (!len) return;
        _ref = new Queue(len);
        while (len--) push(v);
    }
    inline Pipe(const Pipe &from)
    {
        *this = from.ref();
    }
    inline Pipe & operator=(const Pipe &from)
    {
        *this = from.ref();
    }
    inline Pipe(Queue *q = 0) : Reference(q)
    { }
    ~Pipe()
    { }
    const Reference<Queue> &ref()
    {
        if (!_ref) _ref = new Queue(0);
        return *this;
    }
    
    T *push(const T & elem)
    {
        if (!_ref) _ref = new Queue;
        if (!_ref->push(0, sizeof(T))) return 0;
        Slice<uint8_t> d = _ref->data();
        T *t = (T*) d.base();
        size_t len = d.len() / sizeof(T);
        if (t) new (t+=len-1) T(elem);
        return t;
    }
    bool pop(T *data = 0) const
    {
        if (!_ref) return false;
        Slice<uint8_t> d = _ref->data();
        T *t = (T *) d.base();
        size_t len = d.len() / sizeof(T);
        if (!len) return false;
        if (data) *data = t[len-1];
        t[len-1].~T();
        _ref->pop(0, sizeof(T));
        return true;
    }
    
    T *unshift(const T & elem)
    {
        if (!_ref) _ref = new Queue;
        if (!_ref->unshift(0, sizeof(T))) return 0;
        Slice<uint8_t> d = _ref->data();
        T *t = (T*) d.base();
        if (t) new (t) T(elem);
        return t;
    }
    bool shift(T *data = 0) const
    {
        if (!_ref) return false;
        Slice<uint8_t> d = _ref->data();
        T *t = (T *) d.base();
        size_t len = d.len() / sizeof(T);
        if (!len) return false;
        if (data) *data = t[0];
        t[0].~T();
        _ref->shift(0, sizeof(T));
        return true;
    }
    
    Slice<T> data()
    {
        if (!_ref) return Slice<T>(0, 0);
        Slice<uint8_t> r = _ref->data();
        return Slice<T>((T *) r.base(), r.len() / sizeof(T));
    }
};

#endif /* defined(__cplusplus) */

__MPT_NAMESPACE_END

#endif /* _MPT_QUEUE_H */

