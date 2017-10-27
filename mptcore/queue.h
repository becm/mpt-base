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

#ifdef __cplusplus
MPT_STRUCT(decode_queue) : public queue
{
	decode_queue(DataDecoder d = 0) : _dec(d)
	{ }
	~decode_queue()
	{ if (_dec) _dec(&_state, 0, 0); }
	
	ssize_t peek(size_t len, const void *);
	ssize_t receive();
	
	bool encoded() const
	{ return _dec; }
protected:
#else
MPT_STRUCT(decode_queue)
{
#define MPT_DECODE_QUEUE_INIT { MPT_QUEUE_INIT, MPT_DECODE_INIT, 0 }
	MPT_STRUCT(queue) data;
#endif
	MPT_STRUCT(decode_state) _state;
	MPT_TYPE(DataDecoder) _dec;
};

#ifdef __cplusplus
MPT_STRUCT(encode_queue) : public queue
{
	encode_queue(DataEncoder e = 0) : _enc(e)
	{ }
	~encode_queue()
	{ if (_enc) _enc(&_state, 0, 0); }
	
	ssize_t push(size_t len, const void *);
	
	inline size_t done()
	{ return _state.done; }
	
	bool trim(size_t);
	
	bool encoded() const
	{ return _enc; }
protected:
#else
MPT_STRUCT(encode_queue)
{
#define MPT_ENCODE_QUEUE_INIT { MPT_QUEUE_INIT, MPT_ENCODE_INIT, 0 }
	MPT_STRUCT(queue) data;
#endif
	MPT_STRUCT(encode_state) _state;
	MPT_TYPE(DataEncoder) _enc;
};

__MPT_EXTDECL_BEGIN

/* switch memory before and after pivot */
extern int mpt_memrev(void *, size_t , size_t);
/* switch memory content */
extern int mpt_memswap(void *, void *, size_t);

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
extern ssize_t mpt_queue_peek(MPT_STRUCT(decode_queue) *, size_t , void *);
extern ssize_t mpt_queue_recv(MPT_STRUCT(decode_queue) *);
/* send message */
extern ssize_t mpt_queue_push(MPT_STRUCT(encode_queue) *, size_t , const void *);

__MPT_EXTDECL_END

#ifdef __cplusplus

/*!  */
class DecodingQueue : protected decode_queue
{
public:
	DecodingQueue(DataDecoder = 0);
	~DecodingQueue();
	
	bool pendingMessage();
	bool currentMessage(struct message &, struct iovec * = 0);
	bool advance();
protected:
	ssize_t _mlen;
};

/*! interface to raw device */
class IODevice
{
public:
	static int typeIdentifier();
	
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

class Queue : public IODevice
{
public:
	Queue(size_t = 0);
	virtual ~Queue();
	
	/* IODevice interface */
	ssize_t write(size_t , const void *, size_t) __MPT_OVERRIDE;
	ssize_t read(size_t , void *, size_t) __MPT_OVERRIDE;
	
	Slice<uint8_t> peek(size_t = 0) __MPT_OVERRIDE;
	
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
class Pipe
{
public:
	class instance : public Reference<Queue>::instance
	{
	public:
		void unref()
		{
			if (!_ref.lower()) delete this;
		}
	};
	Pipe(const T &v, size_t len)
	{
		if (len && ref().pointer()->prepare(len * sizeof(T))) {
			while (len--) push(v);
		}
	}
	inline Pipe(const Pipe &from)
	{
		*this = from.ref();
	}
	inline Pipe & operator=(const Pipe &from)
	{
		*this = from.ref();
	}
	inline Pipe(instance *q = 0) : _d(q)
	{
		ref();
	}
	~Pipe()
	{
		while (pop());
	}
	const Reference<instance> &ref()
	{
		if (!_d.pointer()) _d.setPointer(new instance);
		return _d;
	}
	bool push(const T & elem)
	{
		Queue *d = ref().pointer();
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		new (t) T(elem);
		if (d->push(t, sizeof(T))) return true;
		t->~T();
		return false;
	}
	bool pop(T *data = 0) const
	{
		Queue *d;
		if (!(d = _d.pointer())) return false;
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		if (!d->pop(t, sizeof(T))) return false;
		if (data) *data = *t;
		else t->~T();
		return true;
	}
	bool unshift(const T & elem)
	{
		Queue *d = ref().pointer();
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		new (t) T(elem);
		if (d->unshift(t, sizeof(T))) return true;
		t->~T();
		return false;
	}
	bool shift(T *data = 0) const
	{
		Queue *d;
		if (!(d = _d.pointer())) return false;
		uint8_t buf[sizeof(T)];
		T *t = static_cast<T *>(static_cast<void *>(buf));
		if (!d->shift(t, sizeof(T))) return false;
		if (data) *data = *t;
		else t->~T();
		return true;
	}
	Slice<T> data()
	{
		Queue *d;
		if (!(d = _d.pointer())) return Slice<T>(0, 0);
		Slice<uint8_t> r = d->peek();
		return Slice<T>((T *) r.base(), r.length() / sizeof(T));
	}
protected:
	Reference<instance> _d;
};

#endif /* defined(__cplusplus) */

__MPT_NAMESPACE_END

#endif /* _MPT_QUEUE_H */
