
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <fnmatch.h>

#include "node.h"
#include "config.h"

extern char **environ;
extern MPT_STRUCT(node) *_mpt_config_root(void);

struct Config {
	MPT_INTERFACE(config) conf;
	MPT_STRUCT(node) *node;
};
static MPT_INTERFACE(metatype) **queryNode(MPT_INTERFACE(config) *ptr, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	struct Config *conf = (void *) ptr;
	MPT_STRUCT(node) *curr;
	
	/* create/change node */
	if (val) {
		MPT_INTERFACE(metatype) *mt;
		size_t len;
		
		if (val->fmt) {
			return 0;
		}
		len = val->ptr ? strlen(val->ptr) + 1 : 0;
		if (!(curr = mpt_node_query(conf->node, (MPT_STRUCT(path) *) path, len))) {
			return 0;
		}
		if (!conf->node) {
			conf->node = curr;
		}
		if (path->len && !(curr = mpt_node_get(curr->children, path))) {
			return 0;
		}
		if (len && !(mt = curr->_meta) && !(curr->_meta = mt = mpt_meta_new(len))) {
			return 0;
		}
		if (mt->_vptr->assign(mt, val) < 0) {
			return 0;
		}
	}
	/* require existing node */
	else if (!(curr = conf->node)
	         || (path->len && !(curr = mpt_node_get(curr, path)))) {
		return 0;
	}
	return &curr->_meta;
}

static const MPT_INTERFACE_VPTR(config) nodeQuery = {
	0, queryNode, 0
};

/*!
 * \ingroup mptConfig
 * \brief set node children from environment.
 * 
 * use pattern matching to only include wanted
 * environment variables.
 * 
 * \param base     root node
 * \param pattern  regular expression
 * \param sep      path separator
 * \param env      special environment settings
 * 
 * \return number of accepted elements
 */
extern int mpt_node_environ(MPT_STRUCT(node) *base, const char *pattern, int sep, char * const env[])
{
	struct Config conf = { { &nodeQuery }, 0 };
	
	if (!base) {
		errno = EFAULT;
		return -1;
	}
	conf.node = base->children;
	
	/* process environment settings */
	sep = mpt_config_environ(&conf.conf, pattern, sep, env);
	base->children = conf.node;
	
	return sep;
}

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
		struct Config conf = { { &nodeQuery }, 0 };
		conf.node = mpt_node_get(0, 0);
		return mpt_config_environ(&conf.conf, pattern, sep, env);
	}
	/* (re)evaluate environment */
	while ((var = *(env++))) {
		MPT_INTERFACE(metatype) **mt, *mp;
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
		path.valid = vlen++;
		
		path.sep = '_';
		path.assign = '=';
		path.flags = 0;
		path.first = 0;
		
		var++;
		accept++;
		
		d.fmt = 0;
		d.ptr = end+1;
		
		if (!(mt = conf->_vptr->query(conf, &path, &d))) {
			errno = EINVAL;
			return -accept;
		}
		if (!(mp = *mt)) {
			*mt = mp = mpt_meta_new(vlen);
			if (!mp || (mp->_vptr->assign(mp, &d) < 0)) {
				errno = ENOTSUP;
				return -accept;
			}
		}
	}
	
	return accept;
}

