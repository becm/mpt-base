/*!
 * general version information
 */

#define _MPT_STRING(x) #x

#if !defined(MPT_RELEASE_MINOR)
# define _MPT_RELEASE(M,m,p) _MPT_STRING(M)
#elif !defined(MPT_RELEASE_TEENY)
# define _MPT_RELEASE(M,m,p) _MPT_STRING(M.m)
#else
# define _MPT_RELEASE(M,m,p) _MPT_STRING(M.m.p)
#endif

#ifdef MPT_RELEASE
# define MPT_VERSION MPT_RELEASE
#elif defined(MPT_RELEASE_MAJOR)
# define MPT_RELEASE _MPT_RELEASE(MPT_RELEASE_MAJOR,MPT_RELEASE_MINOR,MPT_RELEASE_TEENY)
# define MPT_VERSION MPT_RELEASE
#elif defined(__MPT_DATE__)
# define MPT_VERSION "devel ("__MPT_DATE__")"
#else
# define MPT_VERSION "developer build"
#endif

#ifndef SHLIB_VERSION
# ifdef MPT_RELEASE
#  define SHLIB_VERSION "release "MPT_RELEASE
# elif defined(__MPT_DATE__)
#  define SHLIB_VERSION "developer build: "__MPT_DATE__
# else
#  define SHLIB_VERSION "developer build"
# endif
#endif
