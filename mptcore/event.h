/*!
 * MPT core library
 *  event dispatching
 */

#ifndef _MPT_EVENT_H
#define _MPT_EVENT_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(message);

MPT_INTERFACE(output);
MPT_INTERFACE(metatype);


/* single event */
#ifdef _cplusplus
MPT_STRUCT(event)
{
# define MPT_EVENTFLAG(x) x
#else
# define MPT_EVENTFLAG(x) MPT_ENUM(Event##x)
#endif
enum MPT_EVENTFLAG(Flags) {
	MPT_EVENTFLAG(None)       = 0x0,     /* no special operation */
	MPT_EVENTFLAG(Default)    = 0x1,     /* set default event */
	MPT_EVENTFLAG(Fail)       = 0x2,     /* event processing failed */
	MPT_EVENTFLAG(Terminate)  = 0x4,     /* terminate event loop */
	MPT_EVENTFLAG(Flags)      = 0xffff,
	MPT_EVENTFLAG(Retry)      = 0x10000, /* remaining data on input */
	MPT_EVENTFLAG(CtlError)   = 0x20000  /* error in control handler */
};
#ifdef _cplusplus
	event();
	
	int good(const char *);
	int stop(const char *);
	int cont(const char *);
	int term(const char *);
	int fail(const char *, int = -1);
#else

#define MPT_event_good(ev,txt) \
	(mpt_event_reply(ev, 0, "%s", txt), \
	 MPT_EVENTFLAG(None))
#define MPT_event_stop(ev,txt) \
	(mpt_event_reply(ev, 2, "%s", txt), \
	 (ev)->id = 0, MPT_EVENTFLAG(Default))
#define MPT_event_cont(ev,txt) \
	(mpt_event_reply(ev, 2, "%s", txt), \
	 MPT_EVENTFLAG(Default))
#define MPT_event_term(ev,txt) \
	(mpt_event_reply(ev, 3, "%s", txt), \
	 MPT_EVENTFLAG(Terminate))
#define MPT_event_fail(ev,code,txt) \
	(mpt_event_reply(ev, (code), "%s", txt), \
	 (ev)->id = 0, (MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default)))

MPT_STRUCT(event)
{
# define MPT_EVENT_INIT  { 0, 0, { 0, 0 } }
#endif
	uintptr_t                  id;  /* command to process */
	const MPT_STRUCT(message) *msg; /* event data */
	struct {
		int (*set)(void *, const MPT_STRUCT(message) *);
		void *context;
	} reply; /* reply context */
};
typedef int (*MPT_TYPE(EventHandler))(void *, MPT_STRUCT(event) *);

/* event reply dispatcher */
MPT_STRUCT(reply_context)
{
#ifdef __cplusplus
	reply_context() : ptr(0), _max(sizeof(val)), len(0), used(0)
	{ }
	bool active() const
	{ return used; }
	
	void unref();
	
	class array : public RefArray<reply_context>
	{
	public:
		reply_context *reserve(size_t len = 2);
	};
#endif
	void     *ptr;
	uint16_t _max;
	uint8_t   len;
	uint8_t   used;
	uint8_t   val[4];
#ifdef __cplusplus
enum {
# define MPT_REPLY(x) x
#else
};
# define MPT_REPLY(x) MptReply##x
enum MPT_REPLY(Flags) {
#endif
	MPT_REPLY(BadContext)    = 1,
	MPT_REPLY(BadDescriptor) = 2,
	MPT_REPLY(BadState)      = 3,
	MPT_REPLY(BadPush)       = 4
};
#ifdef __cplusplus
};
#endif

/* generic command registration */
MPT_STRUCT(command)
{
#ifdef __cplusplus
private:
	command & operator =(const command &from); /* disable copy */
public:
	inline command() : id(0), cmd(0), arg(0)
	{ }
	inline ~command()
	{ if (cmd) cmd(arg, 0); }
	
	class array : public Array<command>
	{
	public:
		bool set(uintptr_t, EventHandler , void *);
		command *get(uintptr_t);
		command *next(size_t);
	};
#endif
	uintptr_t  id;  /* command id */
	int      (*cmd)(void *, void *);
	void      *arg; /* handler and user supplied data */
};

/* command dispatcher */
#ifdef __cplusplus
MPT_STRUCT(dispatch) : public command::array
{
public:
	dispatch();
	~dispatch();
	
	bool setDefault(uintptr_t);
	void setError(EventHandler , void *);
protected:
#else
MPT_STRUCT(dispatch)
{
# define MPT_DISPATCH_INIT { MPT_ARRAY_INIT, 0, { 0, 0 }, 0 }
	_MPT_ARRAY_TYPE(command) _d;  /* available commands for event */
#endif
	uintptr_t _def; /* default command id */
	struct {
		MPT_TYPE(EventHandler) cmd;
		void *arg;
	} _err;         /* handler for unknown ids */
	
	MPT_STRUCT(reply_context) *_ctx;
};


__MPT_EXTDECL_BEGIN

/* reply with message */
extern int mpt_event_reply(const MPT_STRUCT(event) *, int , const char *, ...);

/* get/set event command */
extern int mpt_command_set(_MPT_ARRAY_TYPE(command) *, const MPT_STRUCT(command) *);
extern MPT_STRUCT(command) *mpt_command_get(const _MPT_ARRAY_TYPE(command) *, uintptr_t);
extern void mpt_command_clear(const _MPT_ARRAY_TYPE(command) *);

/* generate (next) id for message */
extern MPT_STRUCT(command) *mpt_command_nextid(_MPT_ARRAY_TYPE(command) *, size_t);

/* find command with specified id */
extern MPT_STRUCT(command) *mpt_command_find(const MPT_STRUCT(command) *, size_t , uintptr_t);
extern MPT_STRUCT(command) *mpt_command_empty(const MPT_STRUCT(command) *, size_t);


/* initialize/clear dispatcher data */
extern void mpt_dispatch_init(MPT_STRUCT(dispatch) *);
extern void mpt_dispatch_fini(MPT_STRUCT(dispatch) *);

/* register command on event handler */
extern int mpt_dispatch_set(MPT_STRUCT(dispatch) *, uintptr_t , MPT_TYPE(EventHandler) , void *);
/* call event handler and process returned command */
extern int mpt_dispatch_emit(MPT_STRUCT(dispatch) *, MPT_STRUCT(event) *);
/* use id of command string hash */
extern int mpt_dispatch_hash(MPT_STRUCT(dispatch) *, MPT_STRUCT(event) *);

/* register dispatch output control operations */
extern int mpt_dispatch_control(MPT_STRUCT(dispatch) *dsp, const char *, MPT_INTERFACE(output) *);

/* new/available context on array */
extern MPT_STRUCT(reply_context) *mpt_reply_reserve(_MPT_REF_ARRAY_TYPE(reply_context) *arr, size_t len);
/* invalidate/delete context references */
size_t mpt_reply_clear(MPT_STRUCT(reply_context) **, size_t);

/* command message content */
extern MPT_INTERFACE(metatype) *mpt_event_command(const MPT_STRUCT(event) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
class MessageSource
{
public:
    virtual ~MessageSource()
    { }

    virtual const struct message *currentMessage(bool align = false) = 0;
    virtual size_t pendingMessages(int wait = 0) = 0;
    virtual int reply(const struct message * = 0);
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_EVENT_H */
