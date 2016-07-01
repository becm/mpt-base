/*!
 * MPT core library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  @INTERFACE_VERSION@

#include "object.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(notify);
MPT_STRUCT(message);
MPT_STRUCT(msgvalfmt);

enum MPT_ENUM(OutputFlags) {
	MPT_ENUM(OutputPrintNormal)  = 0x1,
	MPT_ENUM(OutputPrintError)   = 0x2,
	MPT_ENUM(OutputPrintHistory) = 0x3,
	MPT_ENUM(OutputPrintRestore) = 0x4,
	MPT_ENUM(OutputPrintColor)   = 0x8,   /* enable coloring */
	
	MPT_ENUM(OutputActive)       = 0x10,  /* message is active */
	MPT_ENUM(OutputRemote)       = 0x20   /* skip internal filter */
};

#ifdef __cplusplus
MPT_INTERFACE(output) : public object
{
protected:
	inline ~output() {}
public:
	enum { Type = TypeOutput };
	
	virtual ssize_t push(size_t, const void *) = 0;
	virtual int sync(int = -1) = 0;
	virtual int await(int (*)(void *, const struct message *) = 0, void * = 0) = 0;
	virtual int log(const char *, int , const char *, va_list) = 0;
	
	int open(const char *);
	int setHistory(const char *);
	int setHistFormat(const char *);
	
	int message(const char *, int, const char *, ... );
};
class Output : public Object
{
public:
	inline Output(output *o) : Object(o)
	{ }
	
};
#else
MPT_INTERFACE(output);
MPT_INTERFACE_VPTR(output)
{
	MPT_INTERFACE_VPTR(object) obj;
	ssize_t (*push)(MPT_INTERFACE(output) *, size_t , const void *);
	int     (*sync)(MPT_INTERFACE(output) *, int);
	int     (*await)(MPT_INTERFACE(output) *, int (*)(void *, const MPT_STRUCT(message) *), void *);
	int     (*log)(MPT_INTERFACE(output) *, const char *, int , const char *, va_list);
};
MPT_INTERFACE(output)
{
	const MPT_INTERFACE_VPTR(output) *_vptr;
};
#endif /* __cplusplus */

/* history information */
MPT_STRUCT(histinfo)
#ifdef _MPT_ARRAY_H
{
# ifdef __cplusplus
public:
	inline histinfo() : pos(0), part(0), line(0), type(0), size(0)
	{ }
	
	bool setFormat(const char *fmt);
	bool setup(size_t , const msgvalfmt *);
protected:
# else
#  define MPT_HISTINFO_INIT  { MPT_ARRAY_INIT,  0, 0, 0,  0, 0 }
# endif
	MPT_STRUCT(array) _fmt;  /* output format */
	uint16_t           pos;  /* position in line */
	uint16_t           part; /* part of line to display */
	uint16_t           line; /* line lenth */
	char               type; /* type information */
	uint8_t            size; /* element size */
}
#endif
;

MPT_STRUCT(outdata)
#ifdef _MPT_ARRAY_H
{
# ifdef __cplusplus
	outdata();
	~outdata();
	
	ssize_t push(size_t, const void *);
protected:
# else
#  define MPT_OUTDATA_INIT { MPT_SOCKET_INIT, 0, MPT_ARRAY_INIT, 0,0,0,0 }
# endif
	MPT_STRUCT(socket) sock;  /* connected datagram socket */
	void *_buf;               /* array buffer or stream pointer */
#if defined(__cplusplus) && defined(_MPT_EVENT_H)
	reply_context::array
#else
	MPT_STRUCT(array)
#endif
	         _ctx;     /* reply context buffer */
	
	uint8_t   state;   /* output state */
	uint8_t  _socklen; /* socket address size */
	uint8_t  _coding;  /* encoding identifier */
	uint8_t  _idlen;   /* identifier length */
}
#endif
;

#if defined(_MPT_ARRAY_H)
# ifdef __cplusplus
MPT_STRUCT(connection) : public outdata
{
public:
	connection();
	~connection();
protected:
# else
MPT_STRUCT(connection)
{
#  define MPT_CONNECTION_INIT { MPT_OUTDATA_INIT, \
                                0, 0, \
                                0, MPT_ARRAY_INIT, \
                                { MPT_HISTINFO_INIT, 0 } }
# endif
	MPT_STRUCT(outdata)  out;   /* base output data */
	uint8_t curr;               /* type of active message */
	uint8_t level;              /* output level */
	
	uintptr_t            cid;   /* active message id */
	MPT_STRUCT(array)   _wait;  /* pending message reply actions */
	
	struct {
	MPT_STRUCT(histinfo) info;  /* history state */
#if defined(_STDIO_H) || defined(_STDIO_H_)
	FILE
#else
	void
#endif
	      *file;
	} hist;         /* history file parameters */
	
};
#else
MPT_STRUCT(connection);
#endif

MPT_STRUCT(output_values)
{
#ifdef __cplusplus
	output_values(uint count, const double *from, int ld = 1);
#else
# define MPT_MSGVAL_INIT { 0, 0, 0, 0 }
#endif
	const void    *base;  /* data base address */
	void         (*copy)(int , const void *, int , void *, int);
	unsigned int   elem;  /* remaining elements */
	int            ld;    /* leading dimension */
};

__MPT_EXTDECL_BEGIN

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);

/* filter control message (open/close), push others */
extern int mpt_output_control(MPT_INTERFACE(output) *, int , const MPT_STRUCT(message) *);

/* history operations */
extern int mpt_history_set(MPT_STRUCT(histinfo) *, const MPT_STRUCT(msgvalfmt) *);
#if defined(_STDIO_H) || defined(_STDIO_H_)
extern ssize_t mpt_history_print(FILE *, MPT_STRUCT(histinfo) *, size_t , const void *);
/* outdata print setup and processing */
extern ssize_t mpt_outdata_print(uint8_t *, FILE *, size_t , const void *);

/* printing values */
extern int mpt_fprint_val(FILE *, MPT_INTERFACE(metatype) *);
#endif

/* determine output print type */
extern int mpt_outdata_type(uint8_t arg, int min);

/* clear outdata */
extern void mpt_outdata_fini(MPT_STRUCT(outdata) *);
/* process return messages */
extern int mpt_outdata_open(MPT_STRUCT(outdata) *, const char *, const char *);
/* clear outdata connection */
extern int mpt_outdata_connect(MPT_STRUCT(outdata) *, const char *, const MPT_STRUCT(fdmode) *);
/* clear outdata connection */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *);
/* get/set outdata property */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *, MPT_STRUCT(property) *);
extern int mpt_outdata_set(MPT_STRUCT(outdata) *, const char *, MPT_INTERFACE(metatype) *);

/* push to outdata */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *, size_t , const void *);
/* process return messages */
extern int mpt_outdata_sync(MPT_STRUCT(outdata) *, const MPT_STRUCT(array) *, int);
/* push message to connection */
extern int mpt_outdata_send(MPT_STRUCT(outdata) *, const MPT_STRUCT(message) *);


/* clear connection data */
extern void mpt_connection_fini(MPT_STRUCT(connection) *);
/* get/set outdata property */
extern int mpt_connection_get(const MPT_STRUCT(connection) *, MPT_STRUCT(property) *);
extern int mpt_connection_set(MPT_STRUCT(connection) *, const char *, MPT_INTERFACE(metatype) *);
/* push to outdata */
extern ssize_t mpt_connection_push(MPT_STRUCT(connection) *, size_t , const void *);
extern int mpt_connection_await(MPT_STRUCT(connection) *, int (*)(void *, const MPT_STRUCT(message) *), void *);
/* handle connection input */
extern int mpt_connection_next(MPT_STRUCT(connection) *, int);
#ifdef _MPT_EVENT_H
extern int mpt_connection_dispatch(MPT_STRUCT(connection) *, MPT_TYPE(EventHandler) cmd, void *arg);
#endif
/* push log message to connection */
extern int mpt_connection_log(MPT_STRUCT(connection) *, const char *, int , const char *, va_list);

/* push output/error message */
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );
/* push value data to putput */
extern int mpt_output_values(MPT_INTERFACE(output) *, const MPT_STRUCT(output_values) *, size_t);

/* create output instance */
extern MPT_INTERFACE(output) *mpt_output_new(MPT_STRUCT(notify) * __MPT_DEFPAR(0));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
