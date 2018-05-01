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
	inline libhandle() : lib(0), create(0)
	{ }
	~libhandle();
#else
# define MPT_LIBHANDLE_INIT { 0, 0 }
#endif
	void *lib;           /* library handle */
	void *(*create)();   /* new/unique instance */
};

__MPT_EXTDECL_BEGIN


/* get input from user */
extern char *mpt_readline(const char *);

/* instance with embedded library handle */
extern MPT_INTERFACE(metatype) *mpt_library_meta(int , const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

/* open/close library descriptor */
extern void *mpt_library_open(const char *, const char *);
/* replace binding if necessary */
extern void *(*mpt_library_symbol(void *, const char *))(void);
/* close library and reset members */
extern const char *mpt_library_close(MPT_STRUCT(libhandle) *);
/* open library handle */
extern int mpt_library_bind(MPT_STRUCT(libhandle) *, const char *, const char *, MPT_INTERFACE(logger) *__MPT_DEFPAR(0));

__MPT_EXTDECL_END

#ifdef __cplusplus
inline libhandle::~libhandle()
{
    mpt_library_close(this);
}
#endif /* C++ */

__MPT_NAMESPACE_END

#endif /* _MPT_CLIENT_H */
