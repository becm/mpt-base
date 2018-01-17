/*!
 * MPT core library
 *  event dispatching
 */

#ifndef _MPT_EVENT_H
#define _MPT_EVENT_H  @INTERFACE_VERSION@

#include "array.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(message);
MPT_STRUCT(notify);

MPT_STRUCT(reply_data)
{
#ifdef __cplusplus
	enum { Type = TypeReplyData };
	
	reply_data(size_t len);
	
	inline bool active() const
	{ return len != 0; }
	bool setData(size_t , const void *);
protected:
#endif
	uint16_t _max;
	uint16_t  len;
	uint8_t   val[4];
};
/* message reply dispatcher */
#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int typeinfo<reply_data *>::id() {
	return reply_data::Type;
}

class reply_context_detached
{
public:
	virtual int reply(const struct message *) = 0;
protected:
	inline ~reply_context_detached()
	{ }
};
#else
MPT_INTERFACE(reply_context_detached);
MPT_INTERFACE_VPTR(reply_context_detached) {
	int (*reply)(MPT_INTERFACE(reply_context_detached) *, const MPT_STRUCT(message) *);
}; MPT_INTERFACE(reply_context_detached) {
	const MPT_INTERFACE_VPTR(reply_context_detached) *_vptr;
};
#endif
#ifdef __cplusplus
class reply_context
{
public:
	enum { Type = TypeReply };
	
	enum {
		MaxSize = 0x100
	};
	
	virtual int reply(const struct message *) = 0;
	virtual reply_context_detached *defer();
protected:
	inline ~reply_context()
	{ }
};
template<> inline __MPT_CONST_TYPE int typeinfo<reply_context *>::id() {
	return reply_context::Type;
}
#else
MPT_INTERFACE(reply_context);
MPT_INTERFACE_VPTR(reply_context) {
	int (*reply)(MPT_INTERFACE(reply_context) *, const MPT_STRUCT(message) *);
	MPT_INTERFACE(reply_context_detached) *(*defer)(MPT_INTERFACE(reply_context) *);
}; MPT_INTERFACE(reply_context) {
	const MPT_INTERFACE_VPTR(reply_context) *_vptr;
};
#endif

/* single event */
#ifdef __cplusplus
MPT_STRUCT(event)
{
	inline event() : reply(0), msg(0), id(0)
	{ }
	int good(const char *);
	int stop(const char *);
	int cont(const char *);
	int term(const char *);
	int fail(const char *, int = -1);
# define MPT_EVENTFLAG(x) x
#else
# define MPT_EVENTFLAG(x) MPT_ENUM(Event_##x)

#define MPT_event_good(ev,txt) \
	(mpt_context_reply(ev->reply, 0, "%s", txt), \
	 MPT_EVENTFLAG(None))
#define MPT_event_stop(ev,txt) \
	(mpt_context_reply(ev->reply, 2, "%s", txt), \
	 (ev)->id = 0, MPT_EVENTFLAG(Default))
#define MPT_event_cont(ev,txt) \
	(mpt_context_reply(ev->reply, 2, "%s", txt), \
	 MPT_EVENTFLAG(Default))
#define MPT_event_term(ev,txt) \
	(mpt_context_reply(ev->reply, 3, "%s", txt), \
	 MPT_EVENTFLAG(Terminate))
#define MPT_event_fail(ev,code,txt) \
	(mpt_context_reply(ev->reply, (code), "%s", txt), \
	 (ev)->id = 0, (MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default)))
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
#ifndef __cplusplus
MPT_STRUCT(event)
{
# define MPT_EVENT_INIT  { 0, 0, 0 }
#endif
	MPT_INTERFACE(reply_context) *reply;  /* reply context */
	const MPT_STRUCT(message) *msg; /* event data */
	uintptr_t id;  /* command to process */
};
typedef int (*MPT_TYPE(EventHandler))(void *, MPT_STRUCT(event) *);

/* generic command registration */
MPT_STRUCT(command)
{
#ifdef __cplusplus
private:
	command & operator =(const command &from); /* disable copy */
public:
	enum { Type = TypeCommand };
	
	inline command() : id(0), cmd(0), arg(0)
	{ }
	inline ~command()
	{ if (cmd) cmd(arg, 0); }
	
	class array : public UniqueArray<command>
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
	
	enum {
		LogStatus = logger::Debug2,
		LogAction = logger::Info
	};
protected:
#else
MPT_STRUCT(dispatch)
{
# define MPT_DISPATCH_LOG_STATUS MPT_LOG(Debug2)
# define MPT_DISPATCH_LOG_ACTION MPT_LOG(Info)
# define MPT_DISPATCH_INIT { MPT_ARRAY_INIT, 0, { 0, 0 }, 0 }
	_MPT_ARRAY_TYPE(command) _d;  /* available commands for event */
#endif
	uintptr_t _def; /* default command id */
	struct {
		MPT_TYPE(EventHandler) cmd;
		void *arg;
	} _err;         /* handler for unknown ids */
	
	MPT_INTERFACE(metatype) *_ctx;  /* fallback reply context */
};


__MPT_EXTDECL_BEGIN

/* reply with message */
extern int mpt_context_reply(MPT_INTERFACE(reply_context) *, int , const char *, ...);

/* get/set event command */
extern int mpt_command_set(_MPT_UARRAY_TYPE(command) *, const MPT_STRUCT(command) *);
extern MPT_STRUCT(command) *mpt_command_get(const _MPT_UARRAY_TYPE(command) *, uintptr_t);
extern void mpt_command_clear(const _MPT_UARRAY_TYPE(command) *);

/* generate (next) id for message */
extern MPT_STRUCT(command) *mpt_command_nextid(_MPT_UARRAY_TYPE(command) *, size_t);

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

/* register config handler on dispatcher */
extern int mpt_dispatch_param(MPT_STRUCT(dispatch) *, MPT_INTERFACE(metatype) * __MPT_DEFPAR(0));

/* create event dispatcher for notifier */
extern MPT_STRUCT(dispatch) *mpt_notify_dispatch(MPT_STRUCT(notify) *);

/* create deferrable reply context */
extern MPT_INTERFACE(metatype) *mpt_reply_deferrable(size_t, int (*)(void *, const MPT_STRUCT(reply_data) *, const MPT_STRUCT(message) *), void *);
/* set reply data */
extern int mpt_reply_set(MPT_STRUCT(reply_data) *, size_t, const void *);

/* command message content */
extern MPT_INTERFACE(metatype) *mpt_event_command(const MPT_STRUCT(event) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
template<> inline __MPT_CONST_TYPE int typeinfo<command>::id()  {
	return command::Type;
}

class MessageSource : public reply_context
{
public:
	virtual ~MessageSource()
	{ }
	virtual const struct message *currentMessage(bool align = false) = 0;
	virtual size_t pendingMessages(int wait = 0) = 0;
};
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_EVENT_H */
