
#include <stdlib.h>
#include <errno.h>

#include "node.h"
#include "config.h"

static MPT_STRUCT(node) __mpt_config_root;

static void config_finish(void)
{
	MPT_STRUCT(node) *curr;
	
	while ((curr = __mpt_config_root.next)) {
		mpt_node_unlink(curr);
		mpt_node_destroy(curr);
	}
	while ((curr = __mpt_config_root.prev)) {
		mpt_node_unlink(curr);
		mpt_node_destroy(curr);
	}
	mpt_node_clear(&__mpt_config_root);
	
	if (__mpt_config_root._meta) {
		__mpt_config_root._meta->_vptr->unref(__mpt_config_root._meta);
		__mpt_config_root._meta = 0;
	}
	mpt_identifier_set(&__mpt_config_root.ident, 0, 0);
}
static MPT_STRUCT(node) *nodeRoot(void)
{
	static MPT_STRUCT(node) *node;
	
	if (node) {
		return node;
	}
	mpt_identifier_init(&__mpt_config_root.ident, sizeof(__mpt_config_root.ident));
	mpt_identifier_set(&__mpt_config_root.ident, "mpt", 3);
	
	atexit(config_finish);
	return node = &__mpt_config_root;
}

/*!
 * \ingroup mptConfig
 * \brief get configuration element
 * 
 * find element in configuration tree/list.
 * 
 * \param conf  configuration root
 * \param path  element path
 * 
 * \return config element if exists
 */
extern MPT_STRUCT(node) *mpt_node_get(MPT_STRUCT(node) *conf, const MPT_STRUCT(path) *path)
{
	MPT_STRUCT(path) p;
	const char *base, *curr;
	ssize_t clen;
	
	if (!path) {
		return conf ? conf : nodeRoot();
	}
	/* missing path information */
	if (!path->len) {
		errno = EINVAL;
		return 0;
	}
	if (!conf) {
		conf = nodeRoot();
	}
	p = *path;
	p.flags &= ~MPT_ENUM(PathHasArray);
	
	base = p.base;
	curr = base + path->off;
	
	while ((clen = mpt_path_next(&p)) >= 0) {
		if (!(conf = mpt_node_locate(conf, 1, curr, clen))) {
			return 0;
		}
		if (!p.len || !(conf = conf->children)) {
			return conf;
		}
		curr = base + p.off;
	}
	return conf;
}
