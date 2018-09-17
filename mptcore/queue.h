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
	{
		return (max - len) < off;
	}
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
	decode_queue(data_decoder_t d = 0) : _dec(d)
	{ }
	~decode_queue()
	{
		if (_dec) _dec(&_state, 0, 0);
	}
	inline bool encoded() const
	{
		return _dec;
	}
	inline bool pending_message() const
	{
		return _state.data.msg >= 0;
	}
	bool current_message(struct message &, struct iovec * = 0) const;
	
	ssize_t peek(size_t len, const void *);
	bool advance();
protected:
#else
MPT_STRUCT(decode_queue)
{
#define MPT_DECODE_QUEUE_INIT { MPT_QUEUE_INIT, MPT_DECODE_INIT, 0 }
	MPT_STRUCT(queue) data;
#endif
	MPT_STRUCT(decode_state) _state;
	MPT_TYPE(data_decoder) _dec;
};

#ifdef __cplusplus
MPT_STRUCT(encode_queue) : public queue
{
	encode_queue(data_encoder_t e = 0) : _enc(e)
	{ }
	~encode_queue()
	{
		if (_enc) _enc(&_state, 0, 0);
	}
	ssize_t push(size_t len, const void *);
	bool trim(size_t);
	
	inline size_t done()
	{
		return _state.done;
	}
	inline bool encoded() const
	{
		return _enc;
	}
protected:
#else
MPT_STRUCT(encode_queue)
{
#define MPT_ENCODE_QUEUE_INIT { MPT_QUEUE_INIT, MPT_ENCODE_INIT, 0 }
	MPT_STRUCT(queue) data;
#endif
	MPT_STRUCT(encode_state) _state;
	MPT_TYPE(data_encoder) _enc;
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
extern int mpt_queue_recv(MPT_STRUCT(decode_queue) *);
extern void mpt_queue_shift(MPT_STRUCT(decode_queue) *);
/* send message */
extern ssize_t mpt_queue_push(MPT_STRUCT(encode_queue) *, size_t , const void *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_QUEUE_H */
