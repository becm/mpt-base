/*!
 * MPT loader library
 *  shared object loading and management
 */

#ifndef _MPT_LOADER_H
#define _MPT_LOADER_H  @INTERFACE_VERSION@

#include "core.h"

__MPT_NAMESPACE_BEGIN

MPT_STRUCT(libhandle)
{
#ifdef __cplusplus
	inline libhandle() : _ref(0), lib(0)
	{ }
	~libhandle();
#else
# define MPT_LIBHANDLE_INIT { { 0 }, 0 }
#endif
	MPT_STRUCT(refcount) _ref;
	void *addr;
};

MPT_STRUCT(libsymbol)
{
#ifdef __cplusplus
	inline libhandle() : lib(0), addr(0), type(0)
	{ }
	~libhandle();
#else
# define MPT_LIBSYMBOL_INIT { 0, 0, 0 }
#endif
	MPT_STRUCT(libhandle) *lib;
	void *addr;   /* symbol address */
	int   type;   /* symbol type */
};

__MPT_EXTDECL_BEGIN


/* get input from user */
extern char *mpt_readline(const char *);

/* instance with embedded library handle */
extern MPT_INTERFACE(metatype) *mpt_library_meta(int , const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* close library descriptor */
extern int mpt_library_open(MPT_STRUCT(libhandle) **, const char *, const char *);
/* reset/detach library */
extern const char *mpt_library_detach(MPT_STRUCT(libhandle) **);

/* replace binding if necessary */
extern int mpt_library_symbol(MPT_STRUCT(libsymbol) *, const char *);
/* open library handle */
extern int mpt_library_bind(MPT_STRUCT(libsymbol) *, const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
inline libhandle::~libhandle()
{
    mpt_library_close(this);
}
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
