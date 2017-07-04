
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "meta.h"
#include "object.h"

#include "config.h"

struct configRoot
{
	MPT_INTERFACE(config) cfg;
	MPT_STRUCT(path)      base;
};

static void configUnref(MPT_INTERFACE(unrefable) *cfg)
{
	struct configRoot *c = (void *) cfg;
	if (c->base.len) {
		free(c);
	}
}
static const MPT_INTERFACE(metatype) *configQuery(const MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path)
{
	struct configRoot *c = (void *) cfg;
	MPT_STRUCT(node) *n;
	MPT_STRUCT(path) p;
	const char *base;
	int len;
	
	if (!(n = mpt_config_node(0))) {
		return 0;
	}
	if (!path) {
		return 0;
	}
	if (c->base.len) {
		p = c->base;
		base = p.base + p.off;
		
		while ((len = mpt_path_next(&p)) >= 0) {
			if (!(n = mpt_node_locate(n, 1, base, len))) {
				return 0;
			}
			if (!p.len && !path->len) {
				return n->_meta;
			}
			if (!(n = n->children)) {
				return 0;
			}
			base = p.base + p.off;
		}
	}
	p = *path;
	base = p.base + p.off;
	
	while ((len = mpt_path_next(&p)) >= 0) {
		if (!(n = mpt_node_locate(n, 1, base, len))) {
			return 0;
		}
		if (!p.len) {
			return n->_meta;
		}
		if (!(n = n->children)) {
			return 0;
		}
		base = p.base + p.off;
	}
	return 0;
}
static int configAssign(MPT_INTERFACE(config) *cfg, const MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	struct configRoot *c = (void *) cfg;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) *n ,*b, *r;
	
	if (!path) {
		return MPT_ERROR(BadArgument);
	}
	if (!(r = b = mpt_config_node(0))) {
		return MPT_ERROR(BadOperation);
	}
	/* get top level node */
	if (c->base.len) {
		MPT_STRUCT(path) p = c->base;
		if (!(b = mpt_node_query(b, &p, 0))) {
			return val ? MPT_ERROR(BadOperation) : 0;
		}
		/* assign top level element */
		if (!path->len) {
			MPT_INTERFACE(metatype) *old;
			if (!val) {
				mt = mpt_metatype_default();
			}
			else if (!(mt = mpt_meta_new(*val))) {
				return MPT_ERROR(BadValue);
			}
			if ((old = b->_meta)) {
				old->_vptr->ref.unref((void *) old);
			}
			b->_meta = mt;
			return mt->_vptr->conv(mt, 0, 0);
		}
		r = b->children;
	}
	/* set subelement */
	if (!(n = mpt_node_assign(&r, path, val))) {
		return val ? MPT_ERROR(BadOperation) : 0;
	}
	if (c->base.len && !b->children) {
		b->children = r;
		r->parent = b;
	}
	mt = n->_meta;
	
	/* return data type identifier */
	return mt ? mt->_vptr->conv(mt, 0, 0) : 0;
}
static const MPT_INTERFACE_VPTR(config) configGlobal = {
	{ configUnref },
	configQuery,
	configAssign,
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
	p.flags &= ~MPT_PATHFLAG(HasArray);
	
	n = mpt_config_node(0);
	if (!(n = mpt_node_query(n, &p, 0))) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		p = *path;
		if (!(n = mpt_node_query(mpt_config_node(0), &p, &val))) {
			return 0;
		}
		if (p.len && !(n = mpt_node_query(n, &p, 0))) {
			return 0;
		}
	}
	if (!(c = malloc(sizeof(*c) + path->len))) {
		return 0;
	}
	c->cfg._vptr = &configGlobal;
	memcpy(&c->base, path, sizeof(c->base));
	memcpy(c + 1, path->base + path->off, path->len);
	
	c->base.base  = (char *) (c + 1);
	c->base.off   = 0;
	
	return &c->cfg;
}
