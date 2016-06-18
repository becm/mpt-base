/*!
 * MPT client library
 *  creation and operations on solver extension
 */

#ifndef _MPT_CLIENT_H
#define _MPT_CLIENT_H  @INTERFACE_VERSION@

#include "config.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(output);

MPT_STRUCT(node);
MPT_STRUCT(event);
MPT_STRUCT(message);
MPT_STRUCT(dispatch);

#ifndef __cplusplus
MPT_INTERFACE(client);
MPT_INTERFACE_VPTR(client)
{
	MPT_INTERFACE_VPTR(config) cfg;
	int  (*init) (MPT_INTERFACE(client) *, MPT_INTERFACE(metatype) *);
	int  (*prep) (MPT_INTERFACE(client) *, MPT_INTERFACE(metatype) *);
	int  (*step) (MPT_INTERFACE(client) *, MPT_INTERFACE(metatype) *);
	void (*clear)(MPT_INTERFACE(client) *);
};
# define MPT_CLIENT_LOGLEVEL MPT_FCNLOG(Debug2)
MPT_INTERFACE(client)
{
	const MPT_INTERFACE_VPTR(client) *_vptr;
#else
MPT_INTERFACE(client) : public config
{
public:
	enum { LogLevel = MPT_FCNLOG(Debug2) };
	
	metatype *query(const path *) const;
	int assign(const path *, const value *);
	int remove(const path *);
	
	virtual int  init(MPT_INTERFACE(metatype) * = 0);
	virtual int  prep(MPT_INTERFACE(metatype) * = 0);
	virtual int  step(MPT_INTERFACE(metatype) * = 0) = 0;
	virtual void clear();
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
#endif
	void *lib;           /* library handle */
	void *(*create)();   /* new/unique instance */
};

MPT_STRUCT(proxy)
{
#ifdef __cplusplus
	class instance
	{
	public:
		virtual void unref() = 0;
	protected:
		inline ~instance() { }
	};
	inline proxy(const char *fmt = 0) : _ref(0)
	{
		size_t i = 0;
		if (fmt) {
			while (*fmt && i < (sizeof(_types) - 1)) {
				_types[i++] = *fmt++;
			}
		}
		while (i < sizeof(_types)) {
			_types[i++] = 0;
		}
	}
	inline ~proxy()
	{
		instance *i = (instance *) _ref;
		if (i) i->unref();
	}
	template <typename T>
	inline T *value() const
	{
		return (T *) mpt_proxy_cast(this, typeIdentifier<T>());
	}
protected:
	instance *_ref;
#else
# define MPT_PROXY_INIT { 0, { 0 } }
	void *_ref;
#endif
	char  _types[sizeof(void *)];
};

__MPT_EXTDECL_BEGIN


/* open connections to controller or standalone run */
extern MPT_STRUCT(notify) *mpt_init(int , char **);

/* get input from user */
extern char *mpt_readline(const char *);


/* initialize/preprare solver, execute solver step */
extern int mpt_cevent_init(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
extern int mpt_cevent_prep(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
extern int mpt_cevent_step(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);

/* register events on notifier */
extern int mpt_client_events(MPT_STRUCT(dispatch) *, MPT_INTERFACE(client) *);

/* set client output */
MPT_INTERFACE(output) *mpt_client_output(MPT_INTERFACE(client) *, MPT_INTERFACE(output) * const *);
/* set/create client logger */
MPT_INTERFACE(logger) *mpt_client_logger(MPT_INTERFACE(client) *, MPT_INTERFACE(object) *);


/* open/close library descriptor */
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *, const char *, const char *);
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *);
/* replace binding if necessary */
extern const char *mpt_library_assign(MPT_STRUCT(libhandle) *, const char *, const char *);

/* interpret type part of library symbol */
int mpt_proxy_type(MPT_STRUCT(proxy) *, const char *);
void *mpt_proxy_cast(const MPT_STRUCT(proxy) *, int);

/* dynamic binding with metatype proxy instance */
MPT_INTERFACE(metatype) *mpt_meta_open(const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));
/* open library handle as metatype */
extern int mpt_library_bind(MPT_STRUCT(proxy) *, const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
inline libhandle::~libhandle()
{
    mpt_library_close(this);
}
inline int client::init(metatype *)
{ return 0; }
inline int client::prep(metatype *)
{ return 0; }
inline void client::clear()
{ }
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
