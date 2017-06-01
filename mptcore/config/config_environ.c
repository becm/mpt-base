
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <fnmatch.h>

#include "node.h"
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
		conf = mpt_config_global(0);
	}
	/* (re)evaluate environment */
	while ((var = *(env++))) {
		MPT_STRUCT(path) path;
		char *end, *pos, tmp[1024];
		MPT_STRUCT(value) d;
		size_t vlen;
		
		if (!(end = strchr(var, '='))) {
			continue;
		}
		path.base = pos = tmp;
		
		/* valid length too big */
		if ((vlen = strlen(end+1)) >= UINT16_MAX) {
			errno = ERANGE;
			continue;
		}
		/* path,sep,value,end > total */
		if (((size_t) (end - var) + 1) >= sizeof(tmp)) {
			errno = ERANGE;
			continue;
		}
		
		/* normalize path */
		while (var < end) { *(pos++) = tolower(*(var++)); }
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

