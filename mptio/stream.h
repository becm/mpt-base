/*!
 * MPT core library
 *  file/network/data interface
 */

#ifndef _MPT_STREAM_H
#define _MPT_STREAM_H  @INTERFACE_VERSION@


#include "array.h"
#include "queue.h"

#ifdef __cplusplus
# include "event.h"
# include "notify.h"
# include "output.h"
#endif

struct sockaddr;

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(message);

MPT_INTERFACE(metatype);

enum MPT_ENUM(StreamFlags) {
	/* state flags */
	MPT_ENUM(StreamRead)     = 0x0,    /* stream is only readable */
	MPT_ENUM(StreamWrite)    = 0x1,    /* stream is only writable */
	MPT_ENUM(StreamRdWr)     = 0x2,    /* stream is read-/writeable */
	
	/* transaction flags */
	MPT_ENUM(StreamMesgAct)   = 0x4,   /* message in progress */
	MPT_ENUM(StreamFlushLine) = 0x8,   /* flush buffer after newline */
	
	/* buffer mode */
	MPT_ENUM(StreamReadBuf)  = 0x10,   /* buffered reading */
	MPT_ENUM(StreamWriteBuf) = 0x20,   /* buffered writing */
	MPT_ENUM(StreamBuffer)   = 0x30,
	MPT_ENUM(StreamReadMap)  = 0x40,   /* read from mapped file */
	MPT_ENUM(StreamWriteMap) = 0x80,   /* write to mapped file */
	
	/* file mode stream flags */
	MPT_ENUM(StreamForceMap)  = 0x100, /* force data mapping */
	MPT_ENUM(StreamFmtDetect) = 0x200  /* detect stream encoding */
};

enum MPT_ENUM(ErrorTypes) {
	MPT_ENUM(ErrorEmpty) = 0x1,        /* input is empty */
	MPT_ENUM(ErrorFull)  = 0x2,        /* output is full */
	MPT_ENUM(ErrorEOF)   = 0x3,
	MPT_ENUM(ErrorRead)  = 0x4,        /* unable to read */
	MPT_ENUM(ErrorWrite) = 0x8         /* unable to write */
};

enum MPT_ENUM(NewlineTypes) {
	MPT_ENUM(NewlineMac)  = 0x1,       /* MacOS line separation */
	MPT_ENUM(NewlineUnix) = 0x2,       /* UNIX line separation */
	MPT_ENUM(NewlineNet)  = 0x3        /* network/Windows line separation */
};

MPT_STRUCT(streaminfo)
{
#ifdef __cplusplus
	inline streaminfo() : _fd(0)
	{ }
	~streaminfo();
	
	bool setFlags(int);
    private:
#else
# define MPT_STREAMINFO_INIT { 0 }
# define MPT_stream_flush(f) (((f) & MPT_ENUM(StreamFlushLine)) ? MPT_stream_newline_write(f) : -1)
#endif
	uintptr_t _fd;  /* file metadata */
};
#define MPT_stream_writable(f)      ((f)  & 0x3)
#define MPT_stream_errors(f)        (((f) & 0xf00 ) >>  8)
#define MPT_stream_newline_read(f)  (((f) & 0x3000) >> 12)
#define MPT_stream_newline_write(f) (((f) & 0xc000) >> 14)


MPT_STRUCT(stream)
{
#ifdef __cplusplus
	stream();
	~stream();
	
	bool endline();
	void setNewline(int , int = StreamRdWr);
	
	int flags() const;
	
	int errors() const;
	void setError(int);
	
	bool open(const char *, const char * = "r");
    protected:
	friend class Stream;
#else
# define MPT_STREAM_INIT { MPT_STREAMINFO_INIT, MPT_DECODE_QUEUE_INIT, MPT_ENCODE_QUEUE_INIT, -1 }
#endif
	MPT_STRUCT(streaminfo)   _info;  /* stream state */
	MPT_STRUCT(decode_queue) _rd;    /* read data */
	MPT_STRUCT(encode_queue) _wd;    /* write data */
	ssize_t                  _mlen;  /* current message length */
};

__MPT_EXTDECL_BEGIN

/* get streaminfo flags */
extern int mpt_stream_flags(const MPT_STRUCT(streaminfo) *);
/* set streaminfo flags */
extern void mpt_stream_seterror(MPT_STRUCT(streaminfo) *, int);
extern void mpt_stream_clearerror(MPT_STRUCT(streaminfo) *, int);
extern void mpt_stream_setnewline(MPT_STRUCT(streaminfo) *, int, int);

/* get socket/input/output file descriptor */
extern int _mpt_stream_fread(const MPT_STRUCT(streaminfo) *);
extern int _mpt_stream_fwrite(const MPT_STRUCT(streaminfo) *);

/* set socket/file descriptor(s) and flags */
extern void _mpt_stream_setalloc(MPT_STRUCT(streaminfo) *);
extern int _mpt_stream_setfile(MPT_STRUCT(streaminfo) *, int , int);
extern pid_t mpt_stream_pipe(MPT_STRUCT(streaminfo) *, const char *, char *const argv[]);

/* change mode (buffering, newline, ...) */
extern int mpt_stream_setmode(MPT_STRUCT(stream) *, int);
/* configure stream via converter function */
extern int mpt_stream_setter(MPT_STRUCT(stream) *, MPT_INTERFACE(metatype) *);

/* connect stream to file/filedescr/process/address/memory */
extern int mpt_stream_open(MPT_STRUCT(stream) *, const char *, const char *);
extern int mpt_stream_dopen(MPT_STRUCT(stream) *, const MPT_STRUCT(socket) *, int);
extern int mpt_stream_memory(MPT_STRUCT(stream) *, const struct iovec *, const struct iovec * __MPT_DEFPAR(0));

/* convert socket connect to stream open flags */
extern int mpt_stream_sockflags(int);

/* accept connection */
extern MPT_INTERFACE(input) *mpt_accept(const MPT_STRUCT(socket) *);
/* create stream interface */
extern MPT_INTERFACE(input) *mpt_stream_input(const MPT_STRUCT(socket) *, int , int , size_t);

/* get next character in stream */
extern int mpt_stream_getc(MPT_STRUCT(stream) *);

/* append/read/seek data to stream */
extern int mpt_stream_poll(MPT_STRUCT(stream) *, int , int);
extern size_t mpt_stream_write(MPT_STRUCT(stream) *, size_t , const void *, size_t);
extern size_t mpt_stream_read(MPT_STRUCT(stream) *, size_t , void *, size_t);
extern int64_t mpt_stream_seek(MPT_STRUCT(stream) *, int64_t , int);

/* finish or add data to current message */
extern ssize_t mpt_stream_push(MPT_STRUCT(stream) *, size_t , const void *);
/* push message and flush stream */
extern int mpt_stream_send(MPT_STRUCT(stream) *, const MPT_STRUCT(message) *);
/* wait for and handle return messages */
extern int mpt_stream_sync(MPT_STRUCT(stream) *, size_t , const _MPT_ARRAY_TYPE(command) *, int __MPT_DEFPAR(-1));
/* dispatch next message */
#ifdef _MPT_EVENT_H
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *, size_t, MPT_TYPE(EventHandler) , void *);
#endif

/* perform delayed write operations */
extern int mpt_stream_flush(MPT_STRUCT(stream) *);
/* close input (and file if valid -> invalidate to avoid) */
extern int mpt_stream_close(MPT_STRUCT(stream) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
struct msgtype;
struct message;

class Stream : public metatype, public output, public input, public IODevice
{
public:
	Stream(const streaminfo * = 0);
	virtual ~Stream();
	
	enum { Type = IODevice::Type };
	
	void unref();
	int conv(int , void *);
	int assign(const value *);
	
	int property(struct property *) const;
	int setProperty(const char *, metatype *);
	
	ssize_t push(size_t , const void *);
	int sync(int = -1);
	int await(int (*)(void *, const struct message *) = 0, void * = 0);
	int log(const char *, int, const char *, va_list);
	
	int next(int);
	int dispatch(EventHandler , void *);
	int _file();
	
	ssize_t write(size_t , const void *, size_t part = 1);
	ssize_t read(size_t , void *, size_t part = 1);
	int64_t pos();
	bool seek(int64_t);
	int getchar();
	
	bool open(const char *, const char * = "r");
	bool open(void *, size_t , int = StreamRead);
	
	inline void close()
	{ setProperty(0, 0); }
	
	class WrapDispatch;
	
protected:
	stream *_srm;
	command::array _wait;
	reply_context::array _ctx;
	uintptr_t _cid;
	int _inputFile;
	uint8_t _idlen;
};
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_STREAM_H */

