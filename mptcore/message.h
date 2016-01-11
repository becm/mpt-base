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
	MPT_ENUM(ValuesUnsigned)  = 0x20,  /* unsigned integer data */
	MPT_ENUM(ValuesFloat)     = 0x40   /* floating point data */
};

enum MPT_ENUM(OutputFlags) {
	MPT_ENUM(OutputLevelNone)      = 0x0,  /* filter messages down to ... */
	MPT_ENUM(OutputLevelCritical)  = 0x1,
	MPT_ENUM(OutputLevelError)     = 0x2,
	MPT_ENUM(OutputLevelWarning)   = 0x3,
	MPT_ENUM(OutputLevelInfo)      = 0x4,
	MPT_ENUM(OutputLevelDebug1)    = 0x5,
	MPT_ENUM(OutputLevelDebug2)    = 0x6,
	MPT_ENUM(OutputLevelDebug3)    = 0x7,
	MPT_ENUM(OutputLevelLog)       = 0x8,
	
	MPT_ENUM(OutputPrintNormal)  = 0x1,
	MPT_ENUM(OutputPrintError)   = 0x2,
	MPT_ENUM(OutputPrintHistory) = 0x3,
	MPT_ENUM(OutputPrintRestore) = 0x4,
	MPT_ENUM(OutputPrintColor)   = 0x8,   /* enable coloring */
	
	MPT_ENUM(OutputActive)       = 0x10,  /* message is active */
	MPT_ENUM(OutputRemote)       = 0x20,  /* skip internal filter */
	
	MPT_ENUM(OutputStateInit)  = 0x1,      /* data states */
	MPT_ENUM(OutputStateStep)  = 0x2,
	MPT_ENUM(OutputStateFini)  = 0x4,
	MPT_ENUM(OutputStateFail)  = 0x8,
	MPT_ENUM(OutputStates)     = 0x7
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

#ifndef __cplusplus
MPT_INTERFACE(output);
MPT_INTERFACE_VPTR(output)
{
	MPT_INTERFACE_VPTR(object) obj;
	ssize_t (*push)(MPT_INTERFACE(output) *, size_t , const void *);
	int     (*sync)(MPT_INTERFACE(output) *, int);
	int     (*await)(MPT_INTERFACE(output) *, int (*)(void *, const MPT_STRUCT(message) *), void *);
};
MPT_INTERFACE(output)
{
	const MPT_INTERFACE_VPTR(output) *_vptr;
};
#else
MPT_INTERFACE(output) : public object
{
protected:
	inline ~output() {}
public:
	enum { Type = TypeOutput };
	
	virtual ssize_t push(size_t, const void *) = 0;
	virtual int sync(int = -1) = 0;
	virtual int await(int (*)(void *, const message *) = 0, void * = 0) = 0;
	
	int open(const char *);
	int setHistory(const char *);
	int setHistFormat(const char *);
	
	int message(const char *, int, const char *, ... );
};
#endif /* __cplusplus */

__MPT_EXTDECL_BEGIN

/* generate (next) id for message */
extern MPT_STRUCT(command) *mpt_message_nextid(MPT_STRUCT(array) *);

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

/* apply property from message text argument */
extern MPT_INTERFACE(metatype) *mpt_meta_message(const MPT_STRUCT(message) *, int , int);

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

/* determine output file type */
extern int mpt_output_file(uint8_t arg, int min);
/* determine message ANSI colour code */
extern const char *mpt_ansi_code(uint8_t);
extern const char *mpt_ansi_reset(void);
/* message type description */
extern const char *mpt_message_identifier(int);
/* determine message level */
int mpt_output_level(const char *);

/* get message on queue */
extern int mpt_message_get(const MPT_STRUCT(queue) *, size_t , size_t , MPT_STRUCT(message) *, struct iovec *);


/* access to output/error functions */
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );
extern MPT_INTERFACE(logger) *mpt_output_logger(const MPT_INTERFACE(output) *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_MESSAGE_H */
