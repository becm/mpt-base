/*!
 * MPT core library
 *  lightweight binary message protocol
 */

#ifndef _MPT_MESSAGE_H
#define _MPT_MESSAGE_H  @INTERFACE_VERSION@

#include "core.h"

#ifdef __cplusplus
# include <stdlib.h>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
# include <sys/endian.h>
#elif defined(sun)
# include <sys/byteorder.h>
#else
# include <endian.h>
#endif

#if !defined(__BYTE_ORDER)
# if !defined(_BYTE_ORDER)
#  error no byte order defined
# else
#  define __BYTE_ORDER     _BYTE_ORDER
#  define __LITTLE_ENDIAN  _LITTLE_ENDIAN
#  define __BIG_ENDIAN     _BIG_ENDIAN
# endif
#endif

#if (__BYTE_ORDER != __LITTLE_ENDIAN) && (__BYTE_ORDER != __BIG_ENDIAN)
# error bad byte order definition
#endif

struct sockaddr;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(queue);
MPT_STRUCT(array);
MPT_STRUCT(node);

MPT_INTERFACE(output);

/* message data */
#ifdef __cplusplus
MPT_STRUCT(message)
{
	inline message(const void *b = 0, size_t u = 0) : used(u), base(b), cont(0), clen(0)
	{ }
	size_t read(size_t len, void * = 0);
	size_t length() const;
#else
# define MPT_MSGVAL(x) MPT_ENUM(Value_##x)
#endif
#ifndef __cplusplus
MPT_STRUCT(message)
{
# define MPT_MESSAGE_INIT { 0, 0, 0, 0 }
#endif
	size_t        used;  /* (verified) message length */
	const void   *base;  /* data base address */
	struct iovec *cont;  /* remaining elements */
	size_t        clen;  /* number of remaining elements */
};

/* interactive message header */
#ifdef __cplusplus
MPT_STRUCT(msgtype)
{
# define MPT_MESGTYPE(x) x
# define MPT_MESGVAL(x)  x
# define MPT_MESGGRF(x)  x
	inline msgtype(int type = Output, int a = 0) : cmd(type), arg(a) { }
#else
# define MPT_MESGTYPE(x) MPT_ENUM(Message_##x)
# define MPT_MESGVAL(x)  MPT_ENUM(MesgVal_##x)
# define MPT_MESGGRF(x)  MPT_ENUM(MesgGrf_##x)
# define MPT_message_value(f,v)  ((sizeof(v) - 1) | MPT_MESGVAL(f) | MPT_MESGVAL(ByteOrderNative))
#endif
enum MPT_MESGTYPE(Type) {
	/* data interaction */
	MPT_MESGTYPE(Output)      = 0x00,  /* send notification */
	MPT_MESGTYPE(Answer)      = 0x01,  /* answer to message */
	
	/* commands and settings */
	MPT_MESGTYPE(Command)     = 0x04,  /* data is simple string command */
	MPT_MESGTYPE(ParamGet)    = 0x05,  /* path delimited by argument */
	MPT_MESGTYPE(ParamSet)    = 0x06,  /* zero-terminated path, argument is depth, remaining is value */
	MPT_MESGTYPE(ParamCond)   = 0x07,  /* set parameter if unset/default */
	
	/* additional headers in message data */
	MPT_MESGTYPE(ValueRaw)    = 0x08,  /* values with type header */
	MPT_MESGTYPE(ValueFmt)    = 0x09,  /* variable length format information */
	MPT_MESGTYPE(Graphic)     = 0x0c,  /* graphic operation */
	MPT_MESGTYPE(Destination) = 0x0d,  /* laydest and msgworld header */
	
	/* extended data transfer */
	MPT_MESGTYPE(UserMin)     = 0x10,  /* start/end number of available types */
	MPT_MESGTYPE(UserMax)     = 0xff
};

enum MPT_MESGTYPE(Value) {
	MPT_MESGVAL(Unsigned)     = 0x20,  /* unsigned integer data */
	MPT_MESGVAL(Float)        = 0x40,  /* floating point data */
	MPT_MESGVAL(Integer)      = 0x60,  /* signed integer data */
	MPT_MESGVAL(Normal)       = 0x60,  /* size =  (val & 0x1f) + 1, unset for big numbers */
	
	MPT_MESGVAL(BigAtom)      = 0x40,  /* size = ((val & 0x1f) + 1) * ValuesBigAtom */
	
	/* explicit little endian and native byte order flag */
	MPT_MESGVAL(ByteOrderLittle) = 0x80,
#if __BYTE_ORDER == __LITTLE_ENDIAN
	MPT_MESGVAL(ByteOrderNative) = 0x80
#else
	MPT_MESGVAL(ByteOrderNative) = 0x00
#endif
};

enum MPT_MESGTYPE(Graphic) {
	MPT_MESGGRF(Flush)        = 0x00,  /* display deferred changes */
	MPT_MESGGRF(Rescale)      = 0x01,  /* rescale operation */
	MPT_MESGGRF(Redraw)       = 0x02,  /* redraw operation */
	
	MPT_MESGGRF(CycleAdvance) = 0x10,  /* advance/reset active cycle */
	
	MPT_MESGGRF(LayoutOpen)   = 0x20,  /* open layout file */
	MPT_MESGGRF(LayoutParse)  = 0x21,  /* parse layout string */
	MPT_MESGGRF(LayoutCreate) = 0x22,  /* create layout */
	MPT_MESGGRF(LayoutClose)  = 0x2f,  /* remove layout */
	
	MPT_MESGGRF(BindingAdd)   = 0x30,  /* add binding */
	MPT_MESGGRF(BindingClear) = 0x31,  /* clear binding */
	MPT_MESGGRF(BindingParse) = 0x32   /* binding in text format */
};
#ifndef __cplusplus
MPT_STRUCT(msgtype)
{
# define MPT_MSGTYPE_INIT { MPT_MESGTYPE(Output), 0 }
#endif
	uint8_t  cmd;  /* message command */
	int8_t   arg;  /* command argument */
};
/* world position */
MPT_STRUCT(msgworld)
{
#ifdef __cplusplus
	inline msgworld() : cycle(0), offset(0) { }
#endif
	int32_t cycle,   /* target cycle (<0: offset from current) */
	        offset;  /* data offset (<0: append) */
};
/* source data type */
#ifdef __cplusplus
MPT_STRUCT(msgbind)
{
	enum {
#define MPT_DATASTATE(x)  x
#else
enum MPT_ENUM(DataStates) {
#define MPT_DATASTATE(x)  MPT_ENUM(DataState_##x)
#endif
	MPT_DATASTATE(Init)  = 0x1,   /* data states */
	MPT_DATASTATE(Step)  = 0x2,
	MPT_DATASTATE(Fini)  = 0x4,
	MPT_DATASTATE(Fail)  = 0x8,
	MPT_DATASTATE(All)   = 0x7
};
#ifdef __cplusplus
	inline msgbind(int d, int s = Init | Step) : dim(d), state(s)
	{ }
#else
MPT_STRUCT(msgbind)
{
# define MPT_MSGBIND_INIT { 0, (MPT_DATASTATE(Init) | MPT_DATASTATE(Step)) }
#endif
	uint8_t dim,    /* source dimension */
	        state;  /* context of data */
};

/* layout destination */
MPT_STRUCT(msgdest)
{
#ifdef __cplusplus
	inline msgdest(uint8_t l = 0, uint8_t g = 0, uint8_t w = 0, uint8_t d = 0) :
		lay(l), grf(g), wld(w), dim(d)
	{ }
	enum {
		MatchLayout    = 1,
		MatchGraph     = 2,
		MatchWorld     = 4,
		MatchPath      = MatchLayout | MatchGraph | MatchWorld,
		MatchDimension = 8,
		MatchAll       = -1
	};
	bool match(msgdest dst, int = MatchAll);
	
	inline bool operator ==(const msgdest &cmp)
	{ return match(cmp); }
#else
# define MPT_MSGDEST_INIT { 0, 0, 0, 0 }
#endif
	uint8_t lay,  /* target layout */
	        grf,  /* target graph */
	        wld,  /* target world */
	        dim;  /* target dimension */
};

MPT_STRUCT(strdest)
{
	uint8_t change,  /* positions which were changed */
	        val[7];  /* values before/after reading */
};

__MPT_EXTDECL_BEGIN

/* push message data to output */
extern int mpt_message_push(MPT_INTERFACE(output) *, const MPT_STRUCT(message) *);

/* get next string parameter from message data */
extern ssize_t mpt_message_argv(MPT_STRUCT(message) *, int );
/* read data from message */
extern size_t mpt_message_read(MPT_STRUCT(message) *, size_t , void *);
/* get message length */
extern size_t mpt_message_length(const MPT_STRUCT(message) *);

/* apply property from message text argument */
extern int mpt_message_pset(MPT_STRUCT(message) *, int , MPT_TYPE(PropertyHandler), void *);

/* create metatype from message text arguments */
extern MPT_INTERFACE(metatype) *mpt_message_iterator(const MPT_STRUCT(message) *, int);

/* set array to message text arguments */
extern int mpt_array_message(MPT_STRUCT(array) *, const MPT_STRUCT(message) *, int);

/* find position of first occurance */
extern ssize_t mpt_memchr(const struct iovec *, size_t , int );
extern ssize_t mpt_memstr(const struct iovec *, size_t , const void *, size_t );
/* find position of last occurance */
extern ssize_t mpt_memrchr(const struct iovec *, size_t , int );
extern ssize_t mpt_memrstr(const struct iovec *, size_t , const void *, size_t );

/* find position from offset accepted by matching function */
extern ssize_t mpt_memfcn (const struct iovec *, size_t , int (*)(int, void *), void *);
extern ssize_t mpt_memrfcn(const struct iovec *, size_t , int (*)(int, void *), void *);

/* find tokens, ignore characters in escape part */
extern ssize_t mpt_memtok(const struct iovec *, size_t , const char *, const char *, const char *);

/* iovec copy operations */
extern ssize_t mpt_memcpy(ssize_t , const struct iovec *, size_t , const struct iovec *, size_t );

/* push message to connection */
extern int mpt_message_append(MPT_STRUCT(array) *, const MPT_STRUCT(message) *);

/* get message on queue */
extern int mpt_message_get(const MPT_STRUCT(queue) *, size_t , size_t , MPT_STRUCT(message) *, struct iovec *);


/* convert message IDs */
extern int mpt_message_id2buf(uint64_t, void *, size_t);
extern int mpt_message_buf2id(const void *, size_t, uint64_t *);


/* parse graphic binding */
extern int mpt_msgbind_set(MPT_STRUCT(msgbind) *, const char *);
/* push bindings to output */
extern int mpt_output_bind_list(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
extern int mpt_output_bind_string(MPT_INTERFACE(output) *, const char *);


/* push (error) message to output */
extern int mpt_output_vlog(MPT_INTERFACE(output) *, const char *, int , const char *, va_list);
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );

/* push raw value header to output */
extern int mpt_output_init_raw(MPT_INTERFACE(output) *, uint8_t);
/* push message value type and destination header to output */
extern int mpt_output_init_plot(MPT_INTERFACE(output) *, MPT_STRUCT(msgdest), uint8_t , const MPT_STRUCT(msgworld) * __MPT_DEFPAR(0));
/* push double values to output */
extern int mpt_output_values(MPT_INTERFACE(output) *, int , const double *, int);
/* send layout open command */
extern int mpt_layout_open(MPT_INTERFACE(output) *, const char *, const char *);

/* parse character separated values */
extern int mpt_string_dest(MPT_STRUCT(strdest) *, int , const char *);

/* get size/type from message value format type */
size_t mpt_msgvalfmt_size(uint8_t);
int mpt_msgvalfmt_type(uint8_t);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_MESSAGE_H */
