
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <fnmatch.h>

#include "meta.h"
#include "types.h"

#include "config.h"

extern char **environ;

/*!
 * \ingroup mptConfig
 * \brief set configuration from environment.
 * 
 * use pattern matching to only include wanted
 * environment variables.
 * 
 * \param conf     configuration target
 * \param pattern  regular expression
 * \param sep      path separator
 * \param env      changed environment variables
 * 
 * \return number of accepted elements
 */
extern int mpt_config_environ(MPT_INTERFACE(config) *conf, const char *pattern, int sep, char * const env[])
{
	const char *var;
	int accept = 0;
	
	if (!env) env = environ;
	if (!sep) sep = '_';
	if (!pattern) pattern = "mpt_*";
	
	/* populate global config */
	if (!conf) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = mpt_config_global(0))
		    || (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &conf) < 0)
		    || !conf) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* (re)evaluate environment */
	while ((var = *(env++))) {
		MPT_STRUCT(path) path;
		char *end, *pos, tmp[1024];
		MPT_STRUCT(value) d;
		
		if (!(end = strchr(var, '='))) {
			continue;
		}
		path.base = pos = tmp;
		
		/* path,sep,value,end > total */
		if (((size_t) (end - var) + 1) >= sizeof(tmp)) {
			errno = ERANGE;
			continue;
		}
		
		/* normalize path */
		while (var < end) {
			*(pos++) = tolower(*(var++));
		}
		*(pos++) = '\0';
		
		/* path matches filter */
		if (fnmatch(pattern, path.base, 0)) {
			continue;
		}
		path.off   = 0;
		path.len   = pos - tmp;
		
		path.sep = sep;
		path.assign = 0;
		path.flags = 0;
		path.first = 0;
		
		var++;
		accept++;
		
		d.fmt = 0;
		d.ptr = end+1;
		
		if (conf->_vptr->assign(conf, &path, &d) < 0) {
			errno = EINVAL;
			return -accept;
		}
	}
	
	return accept;
}

