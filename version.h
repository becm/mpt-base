/*!
 * general version information
 */

#define _TO_STRING(x) #x

#if !defined(RELEASE_MINOR)
# define _MAKE_RELEASE(M,m,p) _TO_STRING(M)
#elif !defined(RELEASE_TEENY)
# define _MAKE_RELEASE(M,m,p) _TO_STRING(M.m)
#else
# define _MAKE_RELEASE(M,m,p) _TO_STRING(M.m.p)
#endif

#ifdef RELEASE_BUILD
# define BUILD_VERSION RELEASE_BUILD
#elif defined(RELEASE_MAJOR)
# define RELEASE_BUILD _MAKE_RELEASE(RELEASE_MAJOR,RELEASE_MINOR,RELEASE_TEENY)
# define BUILD_VERSION RELEASE_BUILD
#elif defined(__VCS_REVISION__)
# define BUILD_VERSION "dev_"__VCS_REVISION__
#elif defined(__ISO_DATE__)
# define BUILD_VERSION "devel ("__ISO_DATE__")"
#else
# define BUILD_VERSION "developer build"
#endif

#ifndef SHLIB_INFO
# ifdef RELEASE_BUILD
#  define SHLIB_INFO "release "RELEASE_BUILD
# elif defined(__VCS_TAG__)
#  define SHLIB_INFO "dev_"__VCS_TAG__
# elif defined(__ISO_DATE__)
#  define SHLIB_INFO "developer build: "__ISO_DATE__
# endif
#endif
