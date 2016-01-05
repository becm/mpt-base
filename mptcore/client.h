/*!
 * MPT client library
 *  creation and operations on solver extension
 */

#ifndef _MPT_CLIENT_H
#define _MPT_CLIENT_H  201502

#include "config.h"

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(output);

MPT_STRUCT(node);
MPT_STRUCT(event);
MPT_STRUCT(message);
MPT_STRUCT(dispatch);

MPT_STRUCT(data);

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
	MPT_INTERFACE(output) *out;
#else
MPT_INTERFACE(client) : public config, public Reference<output>
{
public:
	enum { LogLevel = MPT_FCNLOG(Debug2) };
	
	client(class output * = 0);
	
	Reference<metatype> *query(const path *, const value *);
	int remove(const path *);
	
	virtual int  init(MPT_INTERFACE(metatype) * = 0);
	virtual int  prep(MPT_INTERFACE(metatype) * = 0);
	virtual int  step(MPT_INTERFACE(metatype) * = 0) = 0;
	virtual void clear();
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
	inline proxy() : _mt(0), _id(0)
	{ }
	MPT_INTERFACE(metatype) *meta() const
	{ return _mt; }
	uintptr_t id() const
	{ return _id; }
protected:
	Reference<metatype> _mt;
#else
	MPT_INTERFACE(metatype) *_mt;
#endif
	uintptr_t _id;
};

__MPT_EXTDECL_BEGIN


/* open connections to controller or standalone run */
extern MPT_STRUCT(notify) *mpt_init(int , char **);

/* get input from user */
extern char *mpt_readline(const char *);

/* create solver configuration */
extern MPT_STRUCT(node) *mpt_client_config(const char *);

/* initialize and clear solver data */
extern void mpt_client_init(MPT_INTERFACE(client) *);
extern void mpt_client_fini(MPT_INTERFACE(client) *);


/* initialize/preprare solver, execute solver step */
extern int mpt_cevent_init(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
extern int mpt_cevent_prep(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
extern int mpt_cevent_step(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);

/* register events on notifier */
extern int mpt_client_events(MPT_STRUCT(dispatch) *, MPT_INTERFACE(client) *);

/* read files to configuration */
extern const char *mpt_client_read(MPT_STRUCT(node) *, MPT_STRUCT(message) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));


/* open/close library descriptor */
extern const char *mpt_library_open(MPT_STRUCT(libhandle) *, const char *);
extern const char *mpt_library_assign(MPT_STRUCT(libhandle) *, const char *);
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *);
/* replace binding if necessary */
extern int mpt_library_bind(MPT_STRUCT(proxy) *, const char * , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* dynamic binding with metatype proxy instance */
MPT_INTERFACE(metatype) *mpt_meta_open(const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

__MPT_EXTDECL_END

#ifdef __cplusplus
inline libhandle::~libhandle()
{
    mpt_library_close(this);
}

inline client::client(class output *out) : Reference<class output>(out)
{ }
inline Reference<metatype> *client::query(const path *p, const value *v)
{ return config::global()->query(p, v); }
int client::remove(const path *p)
{ return config::global()->remove(p); }
inline int client::init(metatype *)
{ return 0; }
inline int client::prep(metatype *)
{ return 0; }
inline void client::clear()
{ }
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
