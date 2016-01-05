
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "config.h"

struct configRoot
{
	MPT_INTERFACE(config) cfg;
	MPT_STRUCT(path)      base;
};

static void configUnref(MPT_INTERFACE(config) *cfg)
{
	struct configRoot *c = (void *) cfg;
	if (c->base.len) {
		free(c);
	}
}
static MPT_INTERFACE(metatype) **configQuery(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	struct configRoot *c = (void *) cfg;
	MPT_STRUCT(node) *n ,*b;
	MPT_STRUCT(path) p;
	int len;
	
	if (!(n = b = mpt_config_node(0))) {
		return 0;
	}
	if (!val) {
		len = -1;
	}
	else if (!val->fmt) {
		len = val->ptr ? strlen(val->ptr) + 1 : 0;
	}
	else {
		len = 0;
	}
	if (c->base.len) {
		p = c->base;
		if (!(b = mpt_node_query(b, &p, len))) {
			return 0;
		}
		if (!b->children) {
			b->children = n;
		}
		if (!(n = mpt_node_query(b->children, &p, -1))) {
			return 0;
		}
		if (path) {
			p = *path;
			
			b = n;
			if (!(n = mpt_node_query(n->children, &p, len))) {
				return 0;
			}
			if (!b->children) {
				b->children = n;
			}
			if (p.len && !(n = mpt_node_query(n->children, &p, -1))) {
				return 0;
			}
		}
	}
	else if (path) {
		p = *path;
		if (!(n = mpt_node_query(b, &p, len))) {
			return 0;
		}
		if (p.len && !(n = mpt_node_query(n->children, &p, -1))) {
			return 0;
		}
	}
	if (len > 0 && n->_meta
	    && n->_meta->_vptr->assign(n->_meta, val) < 0) {
		return 0;
	}
	return &n->_meta;
}
static const MPT_INTERFACE_VPTR(config) configGlobal = {
	configUnref,
	configQuery,
	0
};

/*!
 * \ingroup mptConfig
 * \brief get configuration element
 * 
 * find element in global configuration tree/list.
 * 
 * \param path  element path
 * 
 * \return config node if exists
 */
extern MPT_INTERFACE(config) *mpt_config_global(const MPT_STRUCT(path) *path)
{
	struct configRoot *c;
	MPT_STRUCT(path) p;
	MPT_STRUCT(node) *n;
	
	if (!path) {
		static struct configRoot global = { { &configGlobal }, MPT_PATH_INIT };
		return &global.cfg;
	}
	/* missing path information */
	if (!path->len) {
		errno = EINVAL;
		return 0;
	}
	p = *path;
	p.flags &= ~MPT_ENUM(PathHasArray);
	
	n = mpt_config_node(0);
	if (!(n = mpt_node_query(n, &p, 0))) {
		return 0;
	}
	if (p.len && !(n = mpt_node_query(n, &p, 0))) {
		return 0;
	}
	if (!(c = malloc(sizeof(*c) + path->len))) {
		return 0;
	}
	c->cfg._vptr = &configGlobal;
	memcpy(&c->base, path, sizeof(c->base));
	memcpy(c + 1, path->base + path->off, path->len);
	
	c->base.base  = (char *) (c + 1);
	c->base.off   = 0;
	c->base.valid = 0;
	
	return &c->cfg;
}
