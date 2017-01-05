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

enum MPT_ENUM(MessageType) {
	/* data interaction */
	MPT_ENUM(MessageOutput)     = 0x0,   /* send notification */
	MPT_ENUM(MessageAnswer)     = 0x1,   /* answer to message */
	
	/* commands and settings */
	MPT_ENUM(MessageCommand)    = 0x4,   /* data is simple string command */
	MPT_ENUM(MessageParamGet)   = 0x5,   /* path delimited by argument */
	MPT_ENUM(MessageParamSet)   = 0x6,   /* zero-terminated path, argument is depth, remaining is value */
	MPT_ENUM(MessageParamCond)  = 0x7,   /* set parameter if unset/default */
	
	/* additional headers in message data */
	MPT_ENUM(MessageValRaw)     = 0x8,   /* values with type header */
	MPT_ENUM(MessageValFmt)     = 0x9,   /* variable length format information */
	MPT_ENUM(MessageGraphic)    = 0xc,   /* graphic operation */
	MPT_ENUM(MessageDest)       = 0xd,   /* laydest and msgworld header */
	
	/* extended data transfer */
	MPT_ENUM(MessageUserMin)    = 0x10,  /* start/end number of available types */
	MPT_ENUM(MessageUserMax)    = 0xff,
	
#if __BYTE_ORDER == __LITTLE_ENDIAN
	MPT_ENUM(ByteOrderNative)   = 0x80,
#else
	MPT_ENUM(ByteOrderNative)   = 0x0,
#endif
	MPT_ENUM(ByteOrderLittle)   = 0x80
};

enum MPT_ENUM(CommandType) {
	MPT_ENUM(GraphicFlush)    = 0x0,   /* display deferred changes */
	MPT_ENUM(GraphicRescale)  = 0x1,   /* rescale operation */
	MPT_ENUM(GraphicRedraw)   = 0x2,   /* redraw operation */
	
	MPT_ENUM(CycleAdvance)    = 0x10,  /* advance/reset active cycle */
	
	MPT_ENUM(LayoutOpen)      = 0x20,  /* open layout file */
	MPT_ENUM(LayoutParse)     = 0x21,  /* parse layout string */
	MPT_ENUM(LayoutCreate)    = 0x22,  /* create layout */
	MPT_ENUM(LayoutClose)     = 0x2f,  /* remove layout */
	
	MPT_ENUM(BindingAdd)      = 0x30,  /* add binding */
	MPT_ENUM(BindingClear)    = 0x31,  /* clear binding */
	MPT_ENUM(BindingParse)    = 0x32,  /* binding in text format */
	
	MPT_ENUM(ValuesInteger)   = 0x0,   /* signed integer data */
	MPT_ENUM(ValuesBig)       = 0x10,  /* size is 16<<(lower nibble) for float/int/uint */
	MPT_ENUM(ValuesUnsigned)  = 0x20,  /* unsigned integer data */
	MPT_ENUM(ValuesFloat)     = 0x40   /* floating point data */
};

enum MPT_ENUM(DataStates) {
	MPT_ENUM(DataStateInit)   = 0x1,   /* data states */
	MPT_ENUM(DataStateStep)   = 0x2,
	MPT_ENUM(DataStateFini)   = 0x4,
	MPT_ENUM(DataStateFail)   = 0x8,
	MPT_ENUM(DataStateAll)    = 0x7
};

/* message data */
MPT_STRUCT(message)
{
#ifdef __cplusplus
	inline message(const void *b = 0, size_t u = 0) : used(u), base(b), cont(0), clen(0)
	{ }
	size_t read(size_t len, void * = 0);
	size_t length() const;
#else
# define MPT_MESSAGE_INIT { 0, 0, 0, 0 }
#endif
	size_t        used;  /* (verified) message length */
	const void   *base;  /* data base address */
	struct iovec *cont;  /* remaining elements */
	size_t        clen;  /* number of remaining elements */
};

/* interactive message header */
MPT_STRUCT(msgtype)
{
#ifdef __cplusplus
	inline msgtype(int type, int a = 0) : cmd(type), arg(a) { }
#else
# define MPT_MSGTYPE_INIT { 0, 0 }
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
MPT_STRUCT(msgbind)
{
#ifdef __cplusplus
	inline msgbind(int d, int m = DataStateInit | DataStateStep) : dim(d), type(m)
	{ }
#else
# define MPT_MSGBIND_INIT { 0, (MPT_ENUM(OutputStateInit) | MPT_ENUM(OutputStateStep)) }
#endif
	uint8_t dim,   /* source dimension */
	        type;  /* type of data */
};
/* source data type */
MPT_STRUCT(msgvalfmt)
{
#ifdef __cplusplus
	inline msgvalfmt(uint8_t f, unsigned l = 1) : fmt(f), len(l)
	{ }
#else
# define MPT_MSGVALFMT_INIT { 0, 0 } }
#endif
	uint8_t fmt,  /* data type */
	        len;  /* value count */
};

/* layout destination */
MPT_STRUCT(msgdest)
{
#ifdef __cplusplus
	inline msgdest(uint8_t l = 0, uint8_t g = 0, uint8_t w = 0, uint8_t d = 0) :
		lay(l), grf(g), wld(w), dim(d)
	{ }
	inline bool operator==(msgdest ld) const
	{ return lay == ld.lay && grf == ld.grf && wld == ld.wld; }
	inline bool same(msgdest ld) const
	{ return *this == ld && dim == ld.dim; }
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
extern MPT_INTERFACE(metatype) *mpt_meta_message(const MPT_STRUCT(message) *, int);

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

/* value to string conversion */
#if defined(_STDIO_H) || defined(_STDIO_H_)
extern ssize_t mpt_message_print(FILE *, const MPT_STRUCT(message) *);
#endif

/* push message to connection */
extern int mpt_message_append(MPT_STRUCT(array) *, const MPT_STRUCT(message) *);

/* get message on queue */
extern int mpt_message_get(const MPT_STRUCT(queue) *, size_t , size_t , MPT_STRUCT(message) *, struct iovec *);


/* convert message IDs */
extern int mpt_message_id2buf(uint64_t, void *, size_t);
extern int mpt_message_buf2id(const void *, size_t, uint64_t *);

/* send layout open command */
extern int mpt_layout_open(MPT_INTERFACE(output) *, const char *, const char *);

/* parse graphic binding */
extern int mpt_outbind_set(MPT_STRUCT(msgbind) *, const char *);
/* set output bindings */
extern int mpt_outbind_list(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
extern int mpt_outbind_string(MPT_INTERFACE(output) *, const char *);

/* push messages to output */
extern int mpt_output_data(MPT_INTERFACE(output) *, int, int , int , const double *, int);
extern int mpt_output_history(MPT_INTERFACE(output) *, int, const double *, int, const double *, int);
extern int mpt_output_plot(MPT_INTERFACE(output) *, MPT_STRUCT(msgdest), int, const double *, int);

/* parse character separated values */
extern int mpt_string_dest(MPT_STRUCT(strdest) *, int , const char *);

/* get size/type from message value format type */
size_t mpt_msgvalfmt_size(uint8_t);
int mpt_msgvalfmt_type(uint8_t);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_MESSAGE_H */
