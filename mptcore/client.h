/*!
 * MPT client library
 *  creation and operations on solver extension
 */

#ifndef _MPT_CLIENT_H
#define _MPT_CLIENT_H  @INTERFACE_VERSION@

#include "config.h"

#ifdef __cplusplus
# include "meta.h"
# include "output.h"
#endif

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(output);

MPT_STRUCT(node);
MPT_STRUCT(event);
MPT_STRUCT(message);
MPT_STRUCT(dispatch);

MPT_INTERFACE(object);

#ifndef __cplusplus
MPT_INTERFACE(client);
MPT_INTERFACE_VPTR(client)
{
	MPT_INTERFACE_VPTR(config) cfg;
	int  (*init) (MPT_INTERFACE(client) *, MPT_INTERFACE(metatype) *);
	int  (*step) (MPT_INTERFACE(client) *, MPT_INTERFACE(metatype) *);
};
# define MPT_CLIENT_LOG_STATUS MPT_LOG(Debug2)
MPT_INTERFACE(client)
{
	const MPT_INTERFACE_VPTR(client) *_vptr;
#else
MPT_INTERFACE(client) : public config
{
public:
	enum { LogStatus = logger::Debug2 };
	
	metatype *query(const path *) const __MPT_OVERRIDE;
	int assign(const path *, const value *) __MPT_OVERRIDE;
	int remove(const path *) __MPT_OVERRIDE;
	
	virtual int  init(MPT_INTERFACE(metatype) * = 0);
	virtual int  step(MPT_INTERFACE(metatype) * = 0) = 0;
protected:
	inline ~client() { }
#endif
};

MPT_STRUCT(libhandle)
{
#ifdef __cplusplus
	inline libhandle() : lib(0), create(0)
	{ }
	~libhandle();
#else
# define MPT_LIBHANDLE_INIT { 0, 0 }
#endif
	void *lib;           /* library handle */
	void *(*create)();   /* new/unique instance */
};

/* combined references to interface types */
#ifdef __cplusplus
MPT_STRUCT(proxy) : public Reference<metatype>
{
	inline proxy() : _hash(0)
	{ }
	int log(const char *, int , const char *, ...) const;
	
	template <typename T>
	inline T *cast()
	{
		return _ref ? _ref->cast<T>() : 0;
	}
protected:
#else /* __cplusplus */
MPT_STRUCT(proxy)
{
	MPT_INTERFACE(metatype) *_ref;
# define MPT_PROXY_INIT { 0, 0 }
#endif /* __cplusplus */
	uintptr_t _hash;
};

__MPT_EXTDECL_BEGIN

/* get input from user */
extern char *mpt_readline(const char *);

/* initialize/preprare solver, execute solver step */
extern int mpt_cevent_init(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
extern int mpt_cevent_step(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
/* clear config elements */
extern int mpt_cevent_clear(MPT_INTERFACE(config) *, MPT_STRUCT(event) *);

/* register events on notifier */
extern int mpt_client_events(MPT_STRUCT(dispatch) *, MPT_INTERFACE(client) *);

/* set client output */
extern MPT_INTERFACE(output) *mpt_client_output(MPT_INTERFACE(client) *, MPT_INTERFACE(output) * const *);
/* set/create client logger */
extern MPT_INTERFACE(logger) *mpt_client_logger(MPT_INTERFACE(client) *, MPT_INTERFACE(object) *);


/* open/close library descriptor */
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *, const char *, const char *);
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *);
/* replace binding if necessary */
extern const char *mpt_library_assign(MPT_STRUCT(libhandle) *, const char *, const char *);

/* interpret type part of library symbol */
extern int mpt_proxy_type(const char *, const char **);

/* dynamic binding with metatype proxy instance */
extern MPT_INTERFACE(metatype) *mpt_library_meta(const MPT_STRUCT(libhandle) *, int);
/* open library handle as metatype */
extern MPT_INTERFACE(metatype) *mpt_library_bind(uint8_t , const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* clear proxy references */
extern void mpt_proxy_fini(MPT_STRUCT(proxy) *);
/* set matching proxy reference */
extern int mpt_proxy_assign(MPT_STRUCT(proxy) *, const char *, MPT_INTERFACE(metatype) *);
/* try to log to proxy metatype */
extern int mpt_proxy_vlog(const MPT_STRUCT(proxy) *, const char *, int , const char *, va_list);
extern int mpt_proxy_log(const MPT_STRUCT(proxy) *, const char *, int , const char *, ... );

__MPT_EXTDECL_END

#ifdef __cplusplus
inline libhandle::~libhandle()
{
    mpt_library_close(this);
}
inline int client::init(metatype *)
{ return 0; }
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
