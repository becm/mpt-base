/*!
 * MPT core library
 *  output operations
 */

#ifndef _MPT_OUTPUT_H
#define _MPT_OUTPUT_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(property);
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
template<> inline __MPT_CONST_TYPE int typeinfo<output>::id()
{
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


/*! interface to log message */
#ifdef __cplusplus
MPT_INTERFACE(logger)
{
protected:
	inline ~logger() {}
public:
	enum { Type = TypeLogger };
	
	int message(const char *, int , const char *, ...);
	
	static logger *default_instance();
	
	virtual int log(const char *, int, const char *, va_list) = 0;
# define MPT_LOG(x) x
#else
# define MPT_LOG(x) MPT_ENUM(Log##x)
#endif
enum MPT_ENUM(LogType) {
	MPT_LOG(Message)   = 0x0,   /* user (terminal) messages */
	MPT_LOG(Fatal)     = 0x1,
	MPT_LOG(Critical)  = 0x2,
	MPT_LOG(Error)     = 0x3,
	MPT_LOG(Warning)   = 0x4,
	MPT_LOG(Info)      = 0x8,
	MPT_LOG(Debug)     = 0x10,  /* debug level types */
	MPT_LOG(Debug2)    = 0x14,
	MPT_LOG(Debug3)    = 0x18,
	MPT_LOG(File)      = 0x20   /* use log target */
};
enum MPT_ENUM(LogFlags)
{
	MPT_ENUM(LogPrefix)   = 0x100, /* add type prefix */
	MPT_ENUM(LogSelect)   = 0x200, /* use ANSI colouring */
	MPT_ENUM(LogPretty)   = MPT_ENUM(LogPrefix) | MPT_ENUM(LogSelect),
	MPT_ENUM(LogANSIMore) = 0x400, /* no forced ANSI termination */
	
	MPT_ENUM(LogFunction) = 0x800  /* indicate source as function name */
};
#ifdef __cplusplus
};
template<> inline __MPT_CONST_TYPE int typeinfo<logger>::id()
{
	return logger::Type;
}
#else
MPT_INTERFACE(logger);
MPT_INTERFACE_VPTR(logger) {
	int (*log)(MPT_INTERFACE(logger) *, const char *, int , const char *, va_list);
}; MPT_INTERFACE(logger) {
	const MPT_INTERFACE_VPTR(logger) *_vptr;
};
#endif

#ifdef __cplusplus
int critical(const char *, const char *, ... );
int error(const char *, const char *, ... );
int warning(const char *, const char *, ... );
int debug(const char *, const char *, ... );

int println(const char *, ... );

int log(convertable *, const char *, int , const char *, ... );
#endif

/* log file target */
MPT_STRUCT(logfile)
{
#ifdef __cplusplus
	logfile();
	~logfile();
protected:
#else
# define MPT_LOGFILE_INIT  { 0, 0,0, 0,0 }
#endif
#if defined(_STDIO_H) || defined(_STDIO_H_)
	FILE *file;
#else
	void *file;
#endif
	uint8_t state;  /* output state */
	uint8_t mode;   /* output mode */
	
	uint8_t ignore; /* log level settings */
	uint8_t lsep;   /* line sepatator code */
};

__MPT_EXTDECL_BEGIN

/* log message to file */
extern int mpt_logfile_log(MPT_STRUCT(logfile) *, const char *, int , const char *, va_list);
/* push to log file */
extern ssize_t mpt_logfile_push(MPT_STRUCT(logfile) *, size_t , const void *);
/* get/set logfile properties */
extern int mpt_logfile_get(const MPT_STRUCT(logfile) *, MPT_STRUCT(property) *);
extern int mpt_logfile_set(MPT_STRUCT(logfile) *, const char *, MPT_INTERFACE(convertable) *);

/* determine output print flags */
extern int mpt_output_flags(uint8_t arg, int min);

/* determine message ANSI color code */
extern const char *mpt_ansi_code(uint8_t);
extern const char *mpt_ansi_reset(void);


/* try to log to metatype instance */
extern int mpt_convertable_vlog(MPT_INTERFACE(convertable) *, const char *, int , const char *, va_list);
extern int mpt_convertable_log(MPT_INTERFACE(convertable) *, const char *, int , const char *, ... );


/* push (error) message to output */
extern int mpt_output_vlog(MPT_INTERFACE(output) *, const char *, int , const char *, va_list);
extern int mpt_output_log(MPT_INTERFACE(output) *, const char *, int , const char *, ... );

/* push log message */
extern int mpt_log(MPT_INTERFACE(logger) *, const char *, int , const char *, ... );
/* get default logger instance */
extern MPT_INTERFACE(logger) *mpt_log_default(void);


/* push double values to output */
extern int mpt_output_values(MPT_INTERFACE(output) *, int , const double *, int);


/* create remote output instance */
extern MPT_INTERFACE(input) *mpt_output_remote(void);
/* create local output instance */
extern MPT_INTERFACE(metatype) *mpt_output_local(void);


/* set default logger options */
extern int mpt_log_default_format(int);
extern int mpt_log_default_skip(int);

#if defined(_STDIO_H) || defined(_STDIO_H_)
/* start log message */
extern const char *mpt_log_intro(FILE *, int);
#endif

/* message type description */
extern const char *mpt_log_identifier(int);
/* determine message level */
int mpt_log_level(const char *);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_OUTPUT_H */
