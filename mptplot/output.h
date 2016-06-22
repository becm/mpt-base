/*!
 * MPT plotting library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  @INTERFACE_VERSION@

#include "core.h"

#ifdef __cplusplus
# include "message.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(node);
MPT_STRUCT(array);
MPT_STRUCT(notify);
MPT_STRUCT(message);

MPT_INTERFACE(output);

enum MPT_ENUM(OutputFlags) {
	MPT_ENUM(OutputPrintNormal)  = 0x1,
	MPT_ENUM(OutputPrintError)   = 0x2,
	MPT_ENUM(OutputPrintHistory) = 0x3,
	MPT_ENUM(OutputPrintRestore) = 0x4,
	MPT_ENUM(OutputPrintColor)   = 0x8,   /* enable coloring */
	
	MPT_ENUM(OutputActive)       = 0x10,  /* message is active */
	MPT_ENUM(OutputRemote)       = 0x20,  /* skip internal filter */
	
	MPT_ENUM(OutputStateInit)    = 0x1,   /* data states */
	MPT_ENUM(OutputStateStep)    = 0x2,
	MPT_ENUM(OutputStateFini)    = 0x4,
	MPT_ENUM(OutputStateFail)    = 0x8,
	MPT_ENUM(OutputStates)       = 0x7
};

/* source data type */
MPT_STRUCT(msgbind)
{
#ifdef __cplusplus
	inline msgbind(int d, int m = OutputStateInit | OutputStateStep) : dim(d), type(m)
	{ }
#else
# define MPT_MSGBIND_INIT { 0, (MPT_ENUM(OutputStateInit) | MPT_ENUM(OutputStateStep)) }
#endif
	uint8_t dim,   /* source dimension */
	        type;  /* type of data */
};
/* layout destination */
MPT_STRUCT(laydest)
{
#ifdef __cplusplus
	inline laydest(uint8_t l = 0, uint8_t g = 0, uint8_t w = 0, uint8_t d = 0) :
		lay(l), grf(g), wld(w), dim(d)
	{ }
	inline bool operator==(const laydest &ld) const
	{ return lay == ld.lay && grf == ld.grf && wld == ld.wld; }
	inline bool same(const laydest &ld) const
	{ return *this == ld && dim == ld.dim; }
#else
# define MPT_LAYDEST_INIT { 0, 0, 0, 0 }
#endif
	uint8_t lay,  /* target layout */
	        grf,  /* target graph */
	        wld,  /* target world */
	        dim;  /* target dimension */
};

/* history information */
MPT_STRUCT(histinfo)
#ifdef _MPT_ARRAY_H
{
# ifdef __cplusplus
public:
	inline histinfo() : pos(0), part(0), line(0), type(0), size(0)
	{ }
	
	bool setFormat(const char *fmt);
	bool setup(size_t , const msgbind *);
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

#if defined(_MPT_ARRAY_H) && defined(_MPT_QUEUE_H)
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

/* binding to layout mapping */
MPT_STRUCT(mapping)
{
#ifdef __cplusplus
	inline mapping(const msgbind &m = msgbind(0), const laydest &d = laydest(), int c = 0) :
		src(m), client(c), dest(d)
	{ }
	inline bool valid() const
	{ return src.type != 0; }
#else
# define MPT_MAPPING_INIT { MPT_MSGBIND_INIT, 0, MPT_LAYDEST_INIT }
#endif
	MPT_STRUCT(msgbind) src;
	uint16_t            client;
	MPT_STRUCT(laydest) dest;
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

MPT_STRUCT(msgval)
{
#ifdef __cplusplus
	msgval(uint count, const double *from, int ld = 1);
#else
# define MPT_MSGVAL_INIT { 0, 0, 0, 0 }
#endif
	const void    *base;  /* data base address */
	void         (*copy)(int , const void *, int , void *, int);
	unsigned int   elem;  /* remaining elements */
	int            ld;    /* leading dimension */
};

MPT_STRUCT(strdest)
{
	uint8_t change,  /* positions which were changed */
	        val[7];  /* values before/after reading */
};

__MPT_EXTDECL_BEGIN

/* decode string to MPT destination */
extern int mpt_string_dest(MPT_STRUCT(strdest) *, int , const char *);

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);

/* send layout open command */
extern int mpt_layout_open(MPT_INTERFACE(output) *, const char *, const char *);

/* parse graphic binding */
extern int mpt_outbind_set(MPT_STRUCT(msgbind) *, const char *);
/* set output bindings */
extern int mpt_outbind_list(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
extern int mpt_outbind_string(MPT_INTERFACE(output) *, const char *);

/* data output formats */
extern int mpt_output_data(MPT_INTERFACE(output) *, int, int , int , const double *, int);
extern int mpt_output_history(MPT_INTERFACE(output) *, int, const double *, int, const double *, int);
extern int mpt_output_values(MPT_INTERFACE(output) *, const MPT_STRUCT(msgval) *, size_t);
extern int mpt_output_plot(MPT_INTERFACE(output) *, const MPT_STRUCT(laydest) *, int, const double *, int);

/* history operations */
extern int mpt_history_set(MPT_STRUCT(histinfo) *, const MPT_STRUCT(msgbind) *);
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
extern int mpt_outdata_connect(MPT_STRUCT(outdata) *, const char *, MPT_STRUCT(fdmode) *);
/* clear outdata connection */
extern void mpt_outdata_close(MPT_STRUCT(outdata) *);
/* get/set outdata property */
extern int mpt_outdata_get(const MPT_STRUCT(outdata) *, MPT_STRUCT(property) *);
extern int mpt_outdata_set(MPT_STRUCT(outdata) *, const char *, MPT_INTERFACE(metatype) *);
/* push to outdata */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *, size_t , const void *);
/* process return messages */
extern int mpt_outdata_sync(MPT_STRUCT(outdata) *, const MPT_STRUCT(array) *, int);


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
/* send message via connection */
extern int mpt_connection_send(MPT_STRUCT(connection) *, const MPT_STRUCT(message) *);

/* data mapping operations */
extern int mpt_mapping_add(MPT_STRUCT(array) *, const MPT_STRUCT(mapping) *);
extern int mpt_mapping_del(const MPT_STRUCT(array) *, const MPT_STRUCT(msgbind) *, const MPT_STRUCT(laydest) * __MPT_DEFPAR(0), int __MPT_DEFPAR(0));
extern int mpt_mapping_cmp(const MPT_STRUCT(mapping) *, const MPT_STRUCT(msgbind) *, int __MPT_DEFPAR(0));


/* create output instance */
extern MPT_INTERFACE(output) *mpt_output_new(MPT_STRUCT(notify) * __MPT_DEFPAR(0));

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
