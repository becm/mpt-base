/*!
 * MPT client library
 *  creation and operations on solver extension
 */

#ifndef _MPT_CLIENT_H
#define _MPT_CLIENT_H  @INTERFACE_VERSION@

#include "meta.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(event);
MPT_STRUCT(message);

#ifdef __cplusplus
MPT_INTERFACE(client) : public metatype
{
public:
	/* metatype interface */
	int conv(int , void *) const __MPT_OVERRIDE;
	
	/* client extensions */
	virtual int dispatch(event * = 0);
	virtual int process(uintptr_t , iterator * = 0) = 0;
	
	static class config &config();
	
	enum { LogStatus = logger::Debug2 };
protected:
	inline ~client() { }
};
#else
# define MPT_CLIENT_LOG_STATUS MPT_LOG(Debug2)
MPT_INTERFACE(client);
MPT_INTERFACE_VPTR(client)
{
	MPT_INTERFACE_VPTR(metatype) meta;
	int (*dispatch)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
	int (*process)(MPT_INTERFACE(client) *, uintptr_t , MPT_INTERFACE(iterator) *);
}; MPT_INTERFACE(client) {
	const MPT_INTERFACE_VPTR(client) *_vptr;
};
#endif

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

/* get client id */
extern int mpt_client_typeid();

/* process command message */
extern int mpt_client_command(MPT_INTERFACE(client) *, const MPT_STRUCT(message) *, int);

/* open/close library descriptor */
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *, const char *, const char *);
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *);
/* replace binding if necessary */
extern const char *mpt_library_assign(MPT_STRUCT(libhandle) *, const char *, const char *);

/* interpret type part of library symbol */
extern int mpt_proxy_typeid(const char *, const char **);

/* dynamic binding with metatype proxy instance */
extern MPT_INTERFACE(metatype) *mpt_library_meta(const MPT_STRUCT(libhandle) *, int);
/* open library handle as metatype */
extern MPT_INTERFACE(metatype) *mpt_library_bind(const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* clear proxy references */
extern void mpt_proxy_fini(MPT_STRUCT(proxy) *);
/* try to log to proxy metatype */
extern int mpt_proxy_vlog(const MPT_INTERFACE(metatype) *, const char *, int , const char *, va_list);
extern int mpt_proxy_log(const MPT_INTERFACE(metatype) *, const char *, int , const char *, ... );

__MPT_EXTDECL_END

#ifdef __cplusplus
template <> int typeIdentifier<client>();

inline libhandle::~libhandle()
{
    mpt_library_close(this);
}
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
