/*!
 * MPT I/O library
 *  connection definitions
 */

#ifndef _MPT_CONNECTION_H
#define _MPT_CONNECTION_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(property);
MPT_STRUCT(message);

/* collection solver runtime data */
#ifdef __cplusplus
struct command;

MPT_STRUCT(socket)
{
public:
# define MPT_SOCKETFLAG(x) x
#else
# define MPT_SOCKETFLAG(x) MPT_ENUM(Socket##x)
#endif
enum MPT_ENUM(SocketFlags) {
	MPT_SOCKETFLAG(Write)  = 0x1,
	MPT_SOCKETFLAG(Read)   = 0x2,
	MPT_SOCKETFLAG(RdWr)   = 0x3,
	MPT_SOCKETFLAG(Stream) = 0x4
};
#ifdef __cplusplus
	inline socket(int fd = -1) : _id(fd)
	{ }
	~socket();
	
	inline bool active() const
	{
		return _id >= 0;
	}
	
	bool bind(const char *, int = 2);
	bool open(const char *, const char *mode = "w");
	
	bool set(convertable &);
	bool set(const value *);
protected:
#else
MPT_STRUCT(socket)
{
# define MPT_SOCKET_INIT       { -1 }
# define MPT_socket_active(s)  ((s)->_id >= 0)
#endif
	int32_t  _id;     /* socket descriptor */
};

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
	reference<metatype> _rctx;
#else
	MPT_INTERFACE(metatype) *_rctx;
# define MPT_CONNECTION_INIT { MPT_OUTDATA_INIT,  0, MPT_ARRAY_INIT,  0 }
#endif
};

__MPT_EXTDECL_BEGIN

/* socket operations */
extern int mpt_connect(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *);
extern int mpt_bind(MPT_STRUCT(socket) *, const char *, const MPT_STRUCT(fdmode) *, int);

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
extern int mpt_connection_set(MPT_STRUCT(connection) *, const char *, MPT_INTERFACE(convertable) *);

/* push data to connection */
extern ssize_t mpt_connection_push(MPT_STRUCT(connection) *, size_t , const void *);
/* register new id for next message on connection */
extern int mpt_connection_await(MPT_STRUCT(connection) *, int (*)(void *, const MPT_STRUCT(message) *), void *);
/* handle connection input */
extern int mpt_connection_next(MPT_STRUCT(connection) *, int);
/* push log message to connection */
extern int mpt_connection_log(MPT_STRUCT(connection) *, const char *, int , const char *);

#ifdef _MPT_EVENT_H
/* dispatch event to handler */
extern int mpt_connection_dispatch(MPT_STRUCT(connection) *, MPT_TYPE(event_handler) cmd, void *arg);
#endif

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_CONNECTION_H */
