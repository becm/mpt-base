/*!
 * MPT core library
 *  notification framework
 */

#ifndef _MPT_NOTIFY_H
#define _MPT_NOTIFY_H  @INTERFACE_VERSION@

#include "array.h"
#include "event.h"

__MPT_NAMESPACE_BEGIN

#ifdef __cplusplus
MPT_INTERFACE(input) : public unrefable
{
public:
	enum { Type = TypeInput };
	
	virtual int next(int);
	virtual int dispatch(EventHandler , void *);
	virtual int _file();
protected:
	inline ~input() { }
};
# else
MPT_INTERFACE(input);
MPT_INTERFACE_VPTR(input) {
	MPT_INTERFACE_VPTR(unrefable) ref;
	int (*next)(MPT_INTERFACE(input) *, int);
	int (*dispatch)(MPT_INTERFACE(input) *, MPT_TYPE(EventHandler) , void *);
	int (*_file)(MPT_INTERFACE(input) *);
}; MPT_INTERFACE(input) {
	const MPT_INTERFACE_VPTR(input) *_vptr;
};
#endif

/* notification compound */
MPT_STRUCT(notify)
{
#ifdef __cplusplus
public:
	notify();
	~notify();
	
	int wait(int what = -1, int wait = -1);
	bool add(input *);
	
	void setDispatch(dispatch *);
	
	input *next() const;
	size_t used() const { return _fdused; }
	
protected:
	RefArray<input>
#else
	MPT_STRUCT(array)
# define MPT_NOTIFY_INIT { MPT_ARRAY_INIT, MPT_ARRAY_INIT, { 0, 0 }, -1, 0 }
#endif /* __cplusplus */
	             _slot,   /* compound part pointer array */
	             _wait;   /* temporary data for poll info */
	struct {
		MPT_TYPE(EventHandler) cmd;
		void *arg;
	} _disp;              /* dispatch controller */
	
	int          _sysfd;  /* system poll descriptor */
	unsigned int _fdused; /* number of used descriptors */
};

__MPT_EXTDECL_BEGIN

/* initialize/clear poll compound data */
extern void mpt_notify_init(MPT_STRUCT(notify) *);
extern void mpt_notify_fini(MPT_STRUCT(notify) *);

/* poll files in compound */
extern int mpt_notify_wait(MPT_STRUCT(notify) *, int , int);
/* get next slot with pending operation */
extern MPT_INTERFACE(input) *mpt_notify_next(const MPT_STRUCT(notify) *);

/* add/remove input to/from notifier */
extern int mpt_notify_add(MPT_STRUCT(notify) *, int , MPT_INTERFACE(input) *);
extern int mpt_notify_clear(MPT_STRUCT(notify) *, int);

/* check and modify poll compound data size */
extern void *mpt_notify_check(MPT_STRUCT(notify) *, int);

/* add input bound to address */
int mpt_notify_bind(MPT_STRUCT(notify) *, const char *, int __MPT_DEFPAR(2));
/* add input connected to destination */
int mpt_notify_connect(MPT_STRUCT(notify) *, const char *);

/* set event dispatcher for notifier */
extern void mpt_notify_setdispatch(MPT_STRUCT(notify) *, MPT_STRUCT(dispatch) *);

/* execute get/dispatch in loop */
extern int mpt_loop(MPT_STRUCT(notify) *);

__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_NOTIFY_H */
