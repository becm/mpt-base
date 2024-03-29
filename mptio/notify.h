/*!
 * MPT core library
 *  notification framework
 */

#ifndef _MPT_NOTIFY_H
#define _MPT_NOTIFY_H  @INTERFACE_VERSION@

#include "meta.h"
#include "event.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(config);

#ifdef __cplusplus
MPT_INTERFACE(input) : public metatype
{
public:
	virtual int next(int);
	virtual int dispatch(event_handler_t , void *);
protected:
	inline ~input()
	{ }
};
template<> int type_properties<input *>::id(bool);
template<> inline const struct type_traits *type_properties<input *>::traits();

template<> const struct type_traits *type_properties<reference<input> >::traits();

# else
MPT_INTERFACE(input);
MPT_INTERFACE_VPTR(input) {
	MPT_INTERFACE_VPTR(metatype) meta;
	int (*next)(MPT_INTERFACE(input) *, int);
	int (*dispatch)(MPT_INTERFACE(input) *, MPT_TYPE(event_handler) , void *);
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
	
	bool add(input *);
	size_t used() const { return _fdused; }
	
	void set_handler(dispatch *);
	
	int wait(int what = -1, int wait = -1);
	input *next() const;
	
	bool loop();
protected:
#else
# define MPT_NOTIFY_INIT { MPT_ARRAY_INIT, MPT_ARRAY_INIT, { 0, 0 }, -1, 0 }
#endif
	_MPT_REF_ARRAY_TYPE(input) _slot;  /* compound part pointer array */
	_MPT_ARRAY_TYPE(input *)   _wait;  /* temporary data for poll info */
	struct {
		MPT_TYPE(event_handler) cmd;
		void *arg;
	} _disp;              /* dispatch controller */
	
	int          _sysfd;  /* system poll descriptor */
	unsigned int _fdused; /* number of used descriptors */
};

__MPT_EXTDECL_BEGIN

/* initialize/clear poll compound data */
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

/* execute get/dispatch in loop */
extern int mpt_loop(MPT_STRUCT(notify) *);

/* id for registered input metatype */
extern const MPT_STRUCT(named_traits) *mpt_input_type_traits(void);
/* type traits for input reference */
extern const MPT_STRUCT(type_traits) *mpt_input_reference_traits();

/* create input for connect string */
extern MPT_INTERFACE(input) *mpt_input_create(const char *);

/* set connect/listen elements from config */
extern int mpt_notify_config(MPT_STRUCT(notify) *, const MPT_INTERFACE(config) *);


__MPT_EXTDECL_END

__MPT_NAMESPACE_END

#endif /* _MPT_NOTIFY_H */
