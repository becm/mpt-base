/*!
 * MPT core library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  @INTERFACE_VERSION@

#include "event.h"

#ifdef __cplusplus
# include "convert.h"
# include "message.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(property);
MPT_STRUCT(notify);
MPT_STRUCT(message);

#define MPT_OUTPUT_LOGMSG_MAX 256
#ifdef __cplusplus
MPT_INTERFACE(output)
{
protected:
	inline ~output() {}
public:
	enum { Type = TypeOutput };
	
	virtual ssize_t push(size_t, const void *) = 0;
	virtual int sync(int = -1) = 0;
	virtual int await(int (*)(void *, const struct message *) = 0, void * = 0) = 0;
	
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
template<> inline __MPT_CONST_TYPE int typeinfo<output *>::id() {
	return output::Type;
}
#else
MPT_INTERFACE(output);
MPT_INTERFACE_VPTR(output)
{
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
	inline histfmt() : pos(0), fmt(0)
	{ }
	bool setFormat(const char *fmt);
	bool add(valfmt);
	bool add(char);
protected:
#else
# define MPT_HISTFMT_INIT  { MPT_ARRAY_INIT, MPT_ARRAY_INIT, 0,0 }
#endif
	_MPT_ARRAY_TYPE(valfmt) _fmt;  /* output format */
	_MPT_ARRAY_TYPE(char)   _dat;  /* data format */
	
	uint8_t pos;  /* element position */
	char    fmt;  /* data format */
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
	MPT_STRUCT(outdata)        out;   /* output data backend */
	uint32_t                   cid;   /* active message id */
	_MPT_UARRAY_TYPE(command) _wait;  /* pending message reply actions */
	
	/* reply context */
#ifdef __cplusplus
	Reference<metatype> _rctx;
#else
	MPT_INTERFACE(metatype) *_rctx;
# define MPT_CONNECTION_INIT { MPT_OUTDATA_INIT,  0, MPT_ARRAY_INIT,  0 }
#endif
};

__MPT_EXTDECL_BEGIN

/* reset history output state */
void mpt_histfmt_reset(MPT_STRUCT(histfmt) *);

/* clear history resources */
extern void mpt_history_fini(MPT_STRUCT(history) *);
/* get/set history properties */
extern int mpt_history_get(const MPT_STRUCT(history) *, MPT_STRUCT(property) *);
extern int mpt_history_set(MPT_STRUCT(history) *, const char *, const MPT_INTERFACE(metatype) *);

/* push data to history */
extern ssize_t mpt_history_push(MPT_STRUCT(history) *, size_t , const void *);
/* log message to history */
extern int mpt_history_log(MPT_STRUCT(histinfo) *, const char *, int , const char *, va_list);

/* data print setup and processing */
extern ssize_t mpt_history_values(MPT_STRUCT(history) *, size_t , const void *);
/* print to history channel */
extern ssize_t mpt_history_print(MPT_STRUCT(histinfo) *, size_t , const void *);

/* determine output print flags */
extern int mpt_output_flags(uint8_t arg, int min);

/* determine message ANSI color code */
extern const char *mpt_ansi_code(uint8_t);
extern const char *mpt_ansi_reset(void);

/* close outdata connection and buffer */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *);
/* set outdata property */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *, MPT_STRUCT(property) *);
/* assing outdata socket */
extern int mpt_outdata_assign(MPT_STRUCT(outdata) *, const MPT_STRUCT(socket) *);
/* push to outdata */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *, size_t , const void *);
/* process return messages */
extern int mpt_outdata_recv(MPT_STRUCT(outdata) *);
/* send reply message */
extern int mpt_outdata_reply(MPT_STRUCT(outdata) *, size_t, const void *, const MPT_STRUCT(message) *);

/* reset connection data */
extern void mpt_connection_fini(MPT_STRUCT(connection) *);
/* clear connection data */
extern void mpt_connection_close(MPT_STRUCT(connection) *);
/* set new connection target */
extern int mpt_connection_assign(MPT_STRUCT(connection) *, const MPT_STRUCT(socket) *);
extern int mpt_connection_open(MPT_STRUCT(connection) *, const char *, const MPT_STRUCT(fdmode) *);
/* get/set outdata property */
extern int mpt_connection_get(const MPT_STRUCT(connection) *, MPT_STRUCT(property) *);
extern int mpt_connection_set(MPT_STRUCT(connection) *, const char *, const MPT_INTERFACE(metatype) *);

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


/* push (error) message to output */
extern int mpt_output_vlog(MPT_INTERFACE(output) *, const char *, int , const char *, va_list);
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );

/* push double values to output */
extern int mpt_output_values(MPT_INTERFACE(output) *, int , const double *, int);


/* create remote output instance */
extern MPT_INTERFACE(input) *mpt_output_remote(void);
/* create local output instance */
extern MPT_INTERFACE(metatype) *mpt_output_local(void);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
