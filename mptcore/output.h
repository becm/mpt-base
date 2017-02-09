/*!
 * MPT core library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  @INTERFACE_VERSION@

#include "object.h"
#include "event.h"

#ifdef __cplusplus
# include "convert.h"
# include "message.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(notify);
MPT_STRUCT(message);

#define MPT_OUTPUT_LOGMSG_MAX 256
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
	
	int open(const char *);
	int setHistory(const char *);
	int setHistFormat(const char *);
	
	int message(const char *, int, const char *, ... );
# define MPT_OUTFLAG(x) x
#else
# define MPT_OUTFLAG(x) MPT_ENUM(Output##x)
#endif
enum MPT_OUTFLAG(Flags) {
	MPT_OUTFLAG(PrintNormal)  = 0x1,
	MPT_OUTFLAG(PrintError)   = 0x2,
	MPT_OUTFLAG(PrintHistory) = 0x3,
	MPT_OUTFLAG(PrintRestore) = 0x4,
	MPT_OUTFLAG(PrintColor)   = 0x8,   /* enable coloring */
	
	MPT_OUTFLAG(Active)       = 0x10,  /* message is active */
	MPT_OUTFLAG(Received)     = 0x20,  /* data from remote */
	MPT_OUTFLAG(Remote)       = 0x40   /* skip internal filter */
};
#ifdef __cplusplus
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
};
MPT_INTERFACE(output)
{
	const MPT_INTERFACE_VPTR(output) *_vptr;
};
#endif /* __cplusplus */

/* history format information */
MPT_STRUCT(histfmt)
{
#ifdef __cplusplus
public:
	bool setFormat(const char *fmt);
	bool add(valfmt);
	bool add(char);
protected:
#else
# define MPT_HISTFMT_INIT  { MPT_ARRAY_INIT, MPT_ARRAY_INIT, 0,0,0 }
#endif
	_MPT_ARRAY_TYPE(valfmt) _fmt;  /* output format */
	_MPT_ARRAY_TYPE(char)   _dat;  /* data format */
	
	uint16_t pos;  /* element position */
	char     all;  /* default data format */
	char     fmt;  /* data format */
};

MPT_STRUCT(histinfo);
MPT_STRUCT(history);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* history format information */
MPT_STRUCT(histinfo)
{
# ifdef __cplusplus
	histinfo();
	~histinfo();
protected:
# else
#  define MPT_HISTINFO_INIT { 0, 0,0, 0,0 }
# endif
	FILE *file;
	
	uint8_t state;  /* output state */
	uint8_t mode;   /* output mode */
	
	uint8_t ignore; /* log level settings */
	uint8_t lsep;   /* line sepatator code */
};

# ifdef __cplusplus
struct history : public histinfo
{
	history();
	~history();
# else
MPT_STRUCT(history)
{
#  define MPT_HISTORY_INIT { MPT_HISTINFO_INIT, MPT_HISTFMT_INIT }
	MPT_STRUCT(histinfo) info;
# endif
	MPT_STRUCT(histfmt)  fmt;
};
#endif


MPT_STRUCT(outdata)
{
#ifdef __cplusplus
	outdata();
	~outdata();
protected:
#else
# define MPT_OUTDATA_INIT { MPT_ARRAY_INIT, MPT_SOCKET_INIT, 0,0, 0,0 }
#endif
	MPT_STRUCT(array)  buf;   /* array buffer */
	MPT_STRUCT(socket) sock;  /* datagram socket */
	
	uint8_t  state;  /* output state */
	uint8_t _idlen;  /* identifier length */
	
	uint8_t _smax;   /* socket address max */
	uint8_t _scurr;  /* socket address used */
};

MPT_STRUCT(connection)
{
#ifdef __cplusplus
	connection();
	~connection();
protected:
#endif
	MPT_STRUCT(outdata)       out;   /* output data backend */
	uintptr_t                 cid;   /* active message id */
	_MPT_ARRAY_TYPE(command) _wait;  /* pending message reply actions */
	
	/* reply context */
#ifdef __cplusplus
	Reference<reply_context> _rctx;
#else
	MPT_INTERFACE(reply_context) *_rctx;
# define MPT_CONNECTION_INIT { MPT_OUTDATA_INIT, \
                               0, MPT_ARRAY_INIT, \
                               0, \
                               0,0 }
#endif
	uint8_t  pass;   /* limit transfered messages */
	uint8_t  show;   /* print replies to messages */
};

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

/* reset history output state */
void mpt_histfmt_reset(MPT_STRUCT(histfmt) *);

/* clear history resources */
extern void mpt_history_fini(MPT_STRUCT(history) *);
/* push data to history */
extern ssize_t mpt_history_push(MPT_STRUCT(history) *, size_t , const void *);
/* log message to history */
extern int mpt_history_log(MPT_STRUCT(histinfo) *, const char *, int , const char *, va_list);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* outdata print setup and processing */
extern ssize_t mpt_history_values(const MPT_STRUCT(histinfo) *, MPT_STRUCT(histfmt) *, size_t , const void *);

/* partial history output */
extern ssize_t mpt_history_print(MPT_STRUCT(histinfo) *, size_t , const void *);
#endif

/* determine output print type */
extern int mpt_outdata_type(uint8_t arg, int min);

/* clear outdata */
extern void mpt_outdata_fini(MPT_STRUCT(outdata) *);
/* clear outdata connection */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *);
/* get/set outdata property */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *, MPT_STRUCT(property) *);
extern int mpt_outdata_set(MPT_STRUCT(outdata) *, const char *, MPT_INTERFACE(metatype) *);

/* push to outdata */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *, size_t , const void *);
/* process return messages */
extern int mpt_outdata_recv(MPT_STRUCT(outdata) *);
/* send reply message */
extern int mpt_outdata_reply(MPT_STRUCT(outdata) *, const MPT_STRUCT(message) *, size_t, const void *);


/* reset connection data */
extern void mpt_connection_fini(MPT_STRUCT(connection) *);
/* clear connection data */
extern void mpt_connection_close(MPT_STRUCT(connection) *);
/* set new connection target */
extern int mpt_connection_open(MPT_STRUCT(connection) *, const char *, const MPT_STRUCT(fdmode) *);
/* get/set outdata property */
extern int mpt_connection_get(const MPT_STRUCT(connection) *, MPT_STRUCT(property) *);
extern int mpt_connection_set(MPT_STRUCT(connection) *, const char *, MPT_INTERFACE(metatype) *);
/* send reply message */
extern int mpt_connection_reply(MPT_STRUCT(connection) *, const MPT_STRUCT(message) *);

/* push data to connection */
extern ssize_t mpt_connection_push(MPT_STRUCT(connection) *, size_t , const void *);
/* register new id for next message on connection */
extern int mpt_connection_await(MPT_STRUCT(connection) *, int (*)(void *, const MPT_STRUCT(message) *), void *);
/* handle connection input */
extern int mpt_connection_next(MPT_STRUCT(connection) *, int);
/* dispatch event to handler */
extern int mpt_connection_dispatch(MPT_STRUCT(connection) *, MPT_TYPE(EventHandler) cmd, void *arg);
/* push log message to connection */
extern int mpt_connection_log(MPT_STRUCT(connection) *, const char *, int , const char *);

/* push output/error message */
extern int mpt_output_vlog(MPT_INTERFACE(output) *, const char *, int , const char *, va_list);
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );
/* push value data to putput */
extern int mpt_output_values(MPT_INTERFACE(output) *, const MPT_STRUCT(output_values) *, size_t);
/* convert message to printable */
extern int mpt_output_print(MPT_INTERFACE(output) *, const MPT_STRUCT(message) *);

/* create remote output instance */
extern MPT_INTERFACE(metatype) *mpt_output_new(MPT_STRUCT(notify) * __MPT_DEFPAR(0));
/* create local output instance */
extern MPT_INTERFACE(output) *mpt_output_local(MPT_INTERFACE(output) * __MPT_DEFPAR(0));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
